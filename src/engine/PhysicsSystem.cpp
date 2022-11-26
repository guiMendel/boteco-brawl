#include "Collider.h"
#include "PhysicsSystem.h"
#include "GameState.h"
#include <functional>
#include <tuple>

using namespace std;
using namespace SatCollision;

// Gets a line equation for 2 points
pair<float, float> GetLineEquation(Vector2 point1, Vector2 point2);

// Finds out how far along the trajectory the intersection of this collider happened
float GetDistanceAlongTrajectory(Collider &collider, Rectangle trajectoryRectangle, float trajectoryAngle);

void ApplyImpulse(CollisionData collisionData);

// Initial gravity
const Vector2 PhysicsSystem::initialGravity{0, 0.5};

PhysicsSystem::PhysicsSystem(GameState &gameState) : gameState(gameState) {}

void PhysicsSystem::PhysicsUpdate([[maybe_unused]] float deltaTime)
{
  HandleCollisions();
}

// Returns whether the two collider lists have some pair of colliders which are intersecting
// If there is, also populates the collision data struct
bool PhysicsSystem::CheckForCollision(
    vector<shared_ptr<Collider>> colliders1, vector<shared_ptr<Collider>> colliders2, CollisionData &collisionData)
{
  for (auto collider1 : colliders1)
  {
    for (auto collider2 : colliders2)
    {
      auto [distance, normal] = FindMinDistance(collider1->GetBox(), collider2->GetBox(),
                                                collider1->gameObject.GetRotation(), collider2->gameObject.GetRotation());

      // If distance is positive, there is no collision
      if (distance >= 0)
        continue;

      // If one of the colliders is a trigger, simply announce the trigger and carry on
      if (collider1->isTrigger || collider2->isTrigger)
      {
        collider1->gameObject.OnTriggerCollision(collider2->gameObject);
        collider2->gameObject.OnTriggerCollision(collider1->gameObject);

        continue;
      }

      // Populate collision data
      collisionData.source = collider1;
      collisionData.other = collider2;
      collisionData.normal = normal;
      collisionData.penetration = abs(distance);

      return true;
    }
  }

  return false;
}

void PhysicsSystem::HandleCollisions()
{
  // Get validated colliders
  auto validatedColliderStructure = ValidateAllColliders();

  // For each object
  auto objectEntryIterator = validatedColliderStructure.begin();
  while (objectEntryIterator != validatedColliderStructure.end())
  {
    // Check for continuous detection
    auto objectBody = objectEntryIterator->second.at(0)->rigidbodyWeak.lock();

    if (objectBody != nullptr && objectBody->ShouldUseContinuousDetection())
      DetectBetweenFramesCollision(objectEntryIterator, validatedColliderStructure.end());

    else
      DetectCollisions(objectEntryIterator, validatedColliderStructure.end());

    objectEntryIterator++;
  }
}

void PhysicsSystem::DetectCollisions(ValidatedCollidersMap::iterator objectEntryIterator, ValidatedCollidersMap::iterator endIterator)
{
  // Will hold any collision data
  static CollisionData collisionData;

  // Test, for each OTHER object in the list (excluding the ones before this one)
  decltype(objectEntryIterator) otherObjectEntryIterator{objectEntryIterator};
  otherObjectEntryIterator++;

  while (otherObjectEntryIterator != endIterator)
  {
    // Check if they are colliding
    if (CheckForCollision(objectEntryIterator->second, otherObjectEntryIterator->second, collisionData))
      // Resolve collision (apply impulses)
      ResolveCollision(collisionData);

    otherObjectEntryIterator++;
  }
}

void PhysicsSystem::DetectBetweenFramesCollision(ValidatedCollidersMap::iterator objectEntryIterator, ValidatedCollidersMap::iterator endIterator)
{
  cout << "Using continuous collision detection for " << objectEntryIterator->second.at(0)->gameObject.name << endl;

  // Get object body
  auto objectBody = objectEntryIterator->second.at(0)->RequireRigidbody();

  // Get trajectory info
  auto [trajectoryRectangle, trajectoryAngle] = objectBody->GetFrameTrajectory();

  // Holds the collider with which collision happened
  vector<shared_ptr<Collider>> collisionTargets;

  // Holds how far along the displacement axis the collision happened
  float collisionDistance = numeric_limits<float>::max();

  // Dependency callback of the detected collision (callback that must be executed for it to become valid)
  function<void()> collisionDependency;

  // Test, for each OTHER object in the list (excluding the ones before this one)
  decltype(objectEntryIterator) otherObjectEntryIterator{objectEntryIterator};
  otherObjectEntryIterator++;

  while (otherObjectEntryIterator != endIterator)
  {
    // Find out if there is intersection between this other object and our trajectory
    auto [intersectedColliders, intersectionDistance, intersectionDependencyCallback] =
        FindTrajectoryIntersection(
            otherObjectEntryIterator->second, trajectoryRectangle, trajectoryAngle, objectBody->gameObject);

    // If no intersection was found, continue
    if (intersectedColliders.empty())
      continue;

    // We want the collision which must have happened before all others, so:
    // Keep this collider if it's closer to the start of trajectory
    if (intersectionDistance < collisionDistance)
    {
      collisionDistance = intersectionDistance;
      collisionTargets = intersectedColliders;
      collisionDependency = intersectionDependencyCallback;
    }

    otherObjectEntryIterator++;
  }

  // If no between frames collision was detected, carry on to regular collision detection
  if (collisionTargets.empty())
  {
    DetectCollisions(objectEntryIterator, endIterator);
    return;
  }

  // Otherwise, move the body to where collision happened
  objectBody->gameObject.SetPosition(
      objectBody->lastPosition + Vector2::Angled(trajectoryAngle, collisionDistance));

  // Execute this intersection's dependency callback
  collisionDependency();

  // And, finally, resolve the actual collision between this body and the collider
  CollisionData collisionData;

  if (CheckForCollision(objectEntryIterator->second, collisionTargets, collisionData))
    ResolveCollision(collisionData);
  else
    cout << "WARNING: Unable to resolve continuous collision" << endl;
}

vector<shared_ptr<Collider>> PhysicsSystem::ValidateColliders(int id)
{
  // Get colliders
  auto &objectColliders = colliderStructure[id];

  // Will hold verified colliders
  vector<shared_ptr<Collider>> verifiedColliders;

  // Will recalculate rigidbody mass if a collider is removed
  bool colliderWasRemoved{false};

  // For each of it's colliders
  auto colliderIterator = objectColliders.begin();
  while (colliderIterator != objectColliders.end())
  {
    // Remove it if it's expired
    if (colliderIterator->expired())
    {
      colliderIterator = objectColliders.erase(colliderIterator);
      colliderWasRemoved = true;
      continue;
    }

    // Otherwise lock it and add it
    verifiedColliders.push_back(colliderIterator->lock());

    // Advance
    colliderIterator++;
  }

  if (colliderWasRemoved)
  {
    // If it has a rigidbody with auto mass on, derive its new mass
    auto rigidbody = gameState.GetObject(id)->GetComponent<Rigidbody>();

    if (rigidbody != nullptr && rigidbody->UsingAutoMass())
      rigidbody->DeriveMassFromColliders();
  }

  return verifiedColliders;
}

unordered_map<int, vector<shared_ptr<Collider>>> PhysicsSystem::ValidateAllColliders()
{
  unordered_map<int, vector<shared_ptr<Collider>>> verifiedCollidersStructure;

  // For each object entry
  auto objectEntryIterator = colliderStructure.begin();
  while (objectEntryIterator != colliderStructure.end())
  {
    int objectId = objectEntryIterator->first;
    verifiedCollidersStructure[objectId] = ValidateColliders(objectId);

    // If it's empty, remove it from the map
    if (verifiedCollidersStructure[objectId].empty())
    {
      objectEntryIterator = colliderStructure.erase(objectEntryIterator);
      continue;
    }

    objectEntryIterator++;
  }

  return verifiedCollidersStructure;
}

void PhysicsSystem::RegisterCollider(shared_ptr<Collider> collider, int objectId)
{
  if (!collider)
    return;

  colliderStructure[objectId].emplace_back(collider);

  // If it has a rigidbody
  auto rigidbody = gameState.GetObject(objectId)->GetComponent<Rigidbody>();

  if (rigidbody != nullptr)
  {
    // If it has auto mass on, derive its new mass
    if (rigidbody->UsingAutoMass())
      rigidbody->DeriveMassFromColliders();

    // Also recalculate it's smaller dimension
    rigidbody->CalculateSmallestColliderDimension();
  }
}

// Source https://youtu.be/1L2g4ZqmFLQ and https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics6collisionresponse/
void PhysicsSystem::ResolveCollision(CollisionData collisionData)
{
  // If this collision was already dealt with this frame, ignore it
  if (collisionData.source->RequireRigidbody()->IsCollidingWith(*collisionData.other->RequireRigidbody()))
    return;

  ApplyImpulse(collisionData);

  // cout << "Penetration was " << collisionData.penetration << endl;

  // Check if is entering collision
  if (collisionData.source->RequireRigidbody()->WasCollidingWith(*collisionData.other->RequireRigidbody()) == false)
  // if (collisionData.source->WasCollidingWith(*collisionData.other) == false ||
  //     collisionData.penetration < minStayPenetration)
  {
    // Announce collision enter to components
    collisionData.source->gameObject.OnCollisionEnter(collisionData);

    // Switch reference
    std::swap(collisionData.source, collisionData.other);

    // Announce to other object
    collisionData.source->gameObject.OnCollisionEnter(collisionData);
  }

  // Announce collision to components
  collisionData.source->gameObject.OnCollision(collisionData);

  // Switch reference
  std::swap(collisionData.source, collisionData.other);

  // Announce to other object
  collisionData.source->gameObject.OnCollision(collisionData);
}

void ApplyImpulse(CollisionData collisionData)
{
  // Ease of access
  auto bodyA = collisionData.source->RequireRigidbody();
  auto bodyB = collisionData.other->RequireRigidbody();

  // Friction to apply
  float frictionModifier = 1 - min(bodyA->friction, bodyB->friction);

  // Apply it
  bodyA->velocity = bodyA->velocity * frictionModifier;
  bodyB->velocity = bodyB->velocity * frictionModifier;

  // Elasticity of collision
  float elasticity = (bodyA->elasticity + bodyB->elasticity) / 2.0f;

  // Relative velocity
  Vector2 relativeVelocity = bodyA->velocity - bodyB->velocity;

  // Calculate impulse vector along the normal
  float impulseMagnitude = -(1 + elasticity) * Vector2::Dot(relativeVelocity, collisionData.normal) /
                           (bodyA->GetInverseMass() + bodyB->GetInverseMass());

  Vector2 impulse = collisionData.normal * impulseMagnitude;

  // Apply impulse
  bodyA->ApplyImpulse(impulse);
  bodyB->ApplyImpulse(-impulse);

  // Increase penetration a little bit to ensure objects are no longer colliding afterwards
  float penetration = collisionData.penetration;

  bool bodyBStatic = bodyB->type == RigidbodyType::Static;

  // Correct position
  float bodyADisplacement = bodyBStatic ? penetration
                                        : penetration * bodyB->GetMass() / (bodyA->GetMass() + bodyB->GetMass());
  float bodyBDisplacement = penetration - bodyADisplacement;

  // cout << "Body a was at " << (string)bodyA->GetColliders()[0]->GetBox() << endl;

  if (bodyA->type != RigidbodyType::Static)
    bodyA->gameObject.Translate(-collisionData.normal * bodyADisplacement);

  if (bodyBStatic == false)
    bodyB->gameObject.Translate(collisionData.normal * bodyBDisplacement);

  // cout << "Body a is now at " << (string)bodyA->GetColliders()[0]->GetBox() << endl;

  // auto [distance, normal] = FindMinDistance(bodyA->GetColliders()[0]->GetBox(), bodyB->GetColliders()[0]->GetBox(),
  //                                           bodyA->gameObject.GetRotation(), bodyB->gameObject.GetRotation());

  // cout << "New distance is " << distance << endl;
}

auto PhysicsSystem::FindTrajectoryIntersection(ValidatedColliders colliders, Rectangle trajectoryRectangle, float trajectoryAngle, GameObject &sourceObject)
    -> tuple<ValidatedColliders, float, function<void()>>
{
  // TODO: check if colliders are for a body using continuous collision

  // The smallest distance found so far
  float minDistance = numeric_limits<float>::max();

  // Colliders involved in the collision
  ValidatedColliders selectedColliders;

  // For each collider
  for (auto collider : colliders)
  {
    // Detect intersection
    auto [distance, normal] = FindMinDistance(trajectoryRectangle, collider->GetBox(),
                                              trajectoryAngle, collider->gameObject.GetRotation());

    // If distance is positive, there is no collision
    if (distance >= 0)
      continue;

    // If is a trigger, simply trigger it and continue
    if (collider->isTrigger)
    {
      sourceObject.OnTriggerCollision(collider->gameObject);
      collider->gameObject.OnTriggerCollision(sourceObject);

      continue;
    }

    // Get the distance of this collider along the trajectory
    float colliderDistance = GetDistanceAlongTrajectory(
        *collider, trajectoryRectangle, trajectoryAngle);

    // Check if it's got better distance
    if (colliderDistance < minDistance)
    {
      minDistance = colliderDistance;
      selectedColliders = vector{collider};
    }
  }

  return make_tuple(selectedColliders, minDistance, []() {});
}

float GetDistanceAlongTrajectory(Collider &collider, Rectangle trajectoryRectangle, float trajectoryAngle)
{
  // Get line equations for trajectory bounds
  auto boundA = GetLineEquation(
      trajectoryRectangle.TopLeft(trajectoryAngle), trajectoryRectangle.TopRight(trajectoryAngle));
  auto boundB = GetLineEquation(
      trajectoryRectangle.BottomLeft(trajectoryAngle), trajectoryRectangle.BottomRight(trajectoryAngle));

  // Plugs X through a line equation
  auto GetY = [](float x, pair<float, float> line)
  {
    return line.first * x + line.second;
  };

  // Defines a point's position relative to 2 lines
  enum PointPosition
  {
    Smaller,
    Inside,
    Bigger
  };

  // Discovers if a point lies between two lines
  auto PointPositionForLines = [GetY](Vector2 point, pair<float, float> lineA, pair<float, float> lineB)
  {
    // Find y counterparts of the points in the lines
    auto bounds = make_pair(GetY(point.x, lineA), GetY(point.x, lineB));

    // Compare point position
    if (point.y < bounds.first && point.y < bounds.second)
      return Smaller;

    if (point.y > bounds.first && point.y > bounds.second)
      return Bigger;

    return Inside;
  };

  auto GetIntersection = [GetY](pair<float, float> lineA, pair<float, float> lineB)
  {
    float x = (lineA.second - lineB.second) / (lineA.first - lineB.first);
    return Vector2(x, GetY(x, lineA));
  };

  // For each adjacent vertex pair
  auto vertices = collider.GetBox().Vertices();

  // Find the segments of the corresponding faces which lie inside the trajectory, and store their vertices
  vector<Vector2> trimmedFacesVertices;

  for (
      // Start with last & first vertices
      auto vertexA = vertices.end() - 1, vertexB = vertices.begin();
      vertexB != vertices.end();
      vertexA = vertexB++)
  {
    // Check which vertices are inside the trajectory
    bool vertexAPosition = PointPositionForLines(*vertexA, boundA, boundB);
    bool vertexBPosition = PointPositionForLines(*vertexB, boundA, boundB);

    // If both vertices are inside, the face is already good
    if (vertexAPosition == Inside && vertexBPosition == Inside)
    {
      trimmedFacesVertices.push_back(*vertexA);
      trimmedFacesVertices.push_back(*vertexB);
      continue;
    }

    // Otherwise, need to find line equation of this face
    auto faceEquation = GetLineEquation(*vertexA, *vertexB);

    // If both are outside
    if (vertexAPosition != Inside && vertexBPosition != Inside)
    {
      // Ensure they are on opposite sides
      if (vertexAPosition == vertexBPosition)
        continue;

      trimmedFacesVertices.push_back(GetIntersection(boundA, faceEquation));
      trimmedFacesVertices.push_back(GetIntersection(boundB, faceEquation));
      continue;
    }

    // If not, need to know which bound is the smaller
    bool boundASmaller = GetY(0, boundA) < GetY(0, boundB);

    // And which vertex is outside
    auto insideVertex = vertexAPosition == Inside ? *vertexA : *vertexB;

    // If the face crosses the smaller bound
    if (vertexAPosition == Smaller || vertexBPosition == Smaller)
    {
      trimmedFacesVertices.push_back(GetIntersection(boundASmaller ? boundA : boundB, faceEquation));
      trimmedFacesVertices.push_back(insideVertex);
    }

    // Otherwise the face crosses the bigger bound
    trimmedFacesVertices.push_back(GetIntersection(boundASmaller ? boundB : boundA, faceEquation));
    trimmedFacesVertices.push_back(insideVertex);
  }

  // By the end of the loop, we should have all faces which are contained in the trajectory rectangle

  // Get the trajectory displacement vector, normalized
  auto displacementDirection = Vector2::Angled(trajectoryAngle);

  // Will store the minimum distance along the displacement
  float minDistance = numeric_limits<float>::max();

  // For each inside vertex
  for (auto vertex : trimmedFacesVertices)
    minDistance = min(minDistance, Vector2::Dot(vertex, displacementDirection));

  return minDistance;
}

pair<float, float> GetLineEquation(Vector2 point1, Vector2 point2)
{
  float m = (point2.y - point1.y) / (point2.x - point1.x);

  return make_pair(m, point1.y - point1.x * m);
}
