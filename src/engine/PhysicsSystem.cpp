#include "Collider.h"
#include "PhysicsSystem.h"
#include "GameState.h"
#include <functional>
#include <tuple>

using namespace std;
using namespace SatCollision;

// Defines a point's position relative to 2 lines
enum class PointPosition
{
  Smaller,
  Inside,
  Bigger
};

// Plugs X through a line equation
float GetY(float x, pair<float, float> line)
{
  return line.first * x + line.second;
}

// Discovers if a point lies between two lines
PointPosition PointPositionForLines(Vector2 point, pair<float, float> lineA, pair<float, float> lineB)
{
  // Find y counterparts of the points in the lines
  auto bounds = make_pair(GetY(point.x, lineA), GetY(point.x, lineB));

  // Compare point position
  if (point.y < bounds.first && point.y < bounds.second)
    return PointPosition::Smaller;

  if (point.y > bounds.first && point.y > bounds.second)
    return PointPosition::Bigger;

  return PointPosition::Inside;
}

Vector2 GetIntersection(pair<float, float> lineA, pair<float, float> lineB)
{
  float x = (lineA.second - lineB.second) / (lineB.first - lineA.first);
  return Vector2(x, GetY(x, lineA));
}

// Gets a line equation for 2 points
pair<float, float> GetLineEquation(Vector2 point1, Vector2 point2);

// Finds out how far along the trajectory the intersection of this collider happened
float GetDistanceAlongTrajectory(Collider &collider, Rectangle trajectoryRectangle, float trajectoryAngle, GameObject &sourceObject);

void ApplyImpulse(CollisionData collisionData);

// Initial gravity
const Vector2 PhysicsSystem::initialGravity{0, 3};

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
  auto dynamicColliders = ValidateAllColliders(dynamicColliderStructure);
  auto staticColliders = ValidateAllColliders(staticColliderStructure);

  // For each dynamic object
  for (
      auto collidersEntryIterator = dynamicColliders.begin();
      collidersEntryIterator != dynamicColliders.end();
      collidersEntryIterator++)
  {
    Assert(collidersEntryIterator->second.empty() == false, "Collider entry was unexpectedly empty");

    // Check for continuous detection
    auto objectBody = collidersEntryIterator->second.at(0)->RequireRigidbody();

    if (objectBody->ShouldUseContinuousDetection())
    {
      objectBody->printDebug = true;
      DetectBetweenFramesCollision(collidersEntryIterator, dynamicColliders.end(), staticColliders);
    }

    else
    {
      objectBody->printDebug = false;

      DetectCollisions(collidersEntryIterator, dynamicColliders.end(), staticColliders);
    }
  }
}

void PhysicsSystem::DetectCollisions(ValidatedCollidersMap::iterator collidersEntryIterator, ValidatedCollidersMap::iterator endIterator, ValidatedCollidersMap &staticColliders)
{
  // Will hold any collision data
  static CollisionData collisionData;

  // Test, for each OTHER dynamic object in the list (excluding the ones before this one)
  decltype(collidersEntryIterator) otherCollidersEntryIterator{collidersEntryIterator};

  for (otherCollidersEntryIterator++; otherCollidersEntryIterator != endIterator; otherCollidersEntryIterator++)
  {
    // Check if they are colliding
    if (CheckForCollision(collidersEntryIterator->second, otherCollidersEntryIterator->second, collisionData))
      // Resolve collision (apply impulses)
      ResolveCollision(collisionData);
  }

  // Test for all static objects
  for (auto staticEntry : staticColliders)
  {
    // Check if they are colliding
    if (CheckForCollision(collidersEntryIterator->second, staticEntry.second, collisionData))
      // Resolve collision (apply impulses)
      ResolveCollision(collisionData);
  }
}

void PhysicsSystem::DetectBetweenFramesCollision(ValidatedCollidersMap::iterator collidersEntryIterator, ValidatedCollidersMap::iterator endIterator, ValidatedCollidersMap &staticColliders)
{
  // Get object body
  auto objectBody = collidersEntryIterator->second.at(0)->RequireRigidbody();

  // Get trajectory info
  auto [trajectoryRectangle, trajectoryAngle] = objectBody->GetFrameTrajectory();

  // Holds the colliders with which collision happened
  vector<shared_ptr<Collider>> collisionTargets;

  // Holds how far along the displacement axis the collision happened
  float collisionDistance = numeric_limits<float>::max();

  // Dependency callback of the detected collision (callback that must be executed for it to become valid)
  function<void()> collisionDependency;

  // Test, for each OTHER  dynamic object in the list (excluding the ones before this one)
  auto otherCollidersEntryIterator{collidersEntryIterator};

  for (otherCollidersEntryIterator++; otherCollidersEntryIterator != endIterator; otherCollidersEntryIterator++)
  {
    // Find out if there is intersection between this other object and our trajectory
    auto [intersectedColliders, intersectionDistance, intersectionDependencyCallback] =
        FindTrajectoryIntersection(
            otherCollidersEntryIterator->second, trajectoryRectangle, trajectoryAngle, objectBody->gameObject);

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
  }

  // Now test for static bodies
  for (auto staticEntry : staticColliders)
  {
    // Find out if there is intersection between this other object and our trajectory
    auto [intersectedColliders, intersectionDistance, intersectionDependencyCallback] =
        FindTrajectoryIntersection(
            staticEntry.second, trajectoryRectangle, trajectoryAngle, objectBody->gameObject);

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
  }

  // If no between frames collision was detected, carry on to regular collision detection
  if (collisionTargets.empty())
  {
    DetectCollisions(collidersEntryIterator, endIterator, staticColliders);
    return;
  }

  // Make it relative to the object's position
  collisionDistance -= Vector2::Dot(objectBody->lastPosition, Vector2::Angled(trajectoryAngle));

  // Otherwise, move the body to where collision happened
  objectBody->gameObject.SetPosition(
      objectBody->lastPosition + Vector2::Angled(trajectoryAngle, collisionDistance));

  objectBody->intersectionPoints.push_back(objectBody->lastPosition + Vector2::Angled(trajectoryAngle, collisionDistance));

  // Execute this intersection's dependency callback
  collisionDependency();

  // And, finally, resolve the actual collision between this body and the collider
  CollisionData collisionData;

  if (CheckForCollision(collidersEntryIterator->second, collisionTargets, collisionData))
    ResolveCollision(collisionData);
  else
    cout << "WARNING: Unable to resolve continuous collision" << endl;
}

vector<shared_ptr<Collider>> PhysicsSystem::ValidateColliders(int id)
{
  auto objectBody = gameState.GetObject(id)->GetComponent<Rigidbody>();

  bool isDynamic = objectBody != nullptr && objectBody->GetType() == RigidbodyType::Dynamic;

  auto &weakColliders = isDynamic ? dynamicColliderStructure : staticColliderStructure;

  // Get colliders
  auto &objectColliders = weakColliders[id];

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

    if (rigidbody != nullptr)
    {
      if (rigidbody->UsingAutoMass())
        rigidbody->DeriveMassFromColliders();

      rigidbody->CalculateSmallestColliderDimension();
    }
  }

  return verifiedColliders;
}

unordered_map<int, vector<shared_ptr<Collider>>> PhysicsSystem::ValidateAllColliders(unordered_map<int, WeakColliders> &weakColliders)
{
  unordered_map<int, vector<shared_ptr<Collider>>> verifiedCollidersStructure;

  // For each object entry
  auto collidersEntryIterator = weakColliders.begin();
  while (collidersEntryIterator != weakColliders.end())
  {
    int objectId = collidersEntryIterator->first;
    verifiedCollidersStructure[objectId] = ValidateColliders(objectId);

    // If it's empty, remove it from the map
    if (verifiedCollidersStructure[objectId].empty())
    {
      collidersEntryIterator = weakColliders.erase(collidersEntryIterator);
      verifiedCollidersStructure.erase(objectId);
      continue;
    }

    collidersEntryIterator++;
  }

  return verifiedCollidersStructure;
}

void PhysicsSystem::RegisterCollider(shared_ptr<Collider> collider, int objectId)
{
  if (!collider)
    return;

  // Get rigidbody if it exists
  auto rigidbody = gameState.GetObject(objectId)->GetComponent<Rigidbody>();

  bool isDynamic{false};

  // Check if it's dynamic
  if (rigidbody != nullptr)
    isDynamic = rigidbody->GetType() == RigidbodyType::Dynamic;

  if (isDynamic)
    dynamicColliderStructure[objectId].emplace_back(collider);
  else
    staticColliderStructure[objectId].emplace_back(collider);

  // (isDynamic ? dynamicColliderStructure : staticColliderStructure)[objectId].emplace_back(collider);

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

  // Check if is entering collision
  if (collisionData.source->RequireRigidbody()->WasCollidingWith(*collisionData.other->RequireRigidbody()) == false)
  // if (collisionData.source->WasCollidingWith(*collisionData.other) == false ||
  //     collisionData.penetration < minStayPenetration)
  {
    // Announce collision enter to components
    collisionData.source->gameObject.OnCollisionEnter(collisionData);

    // Switch reference
    swap(collisionData.source, collisionData.other);

    // Announce to other object
    collisionData.source->gameObject.OnCollisionEnter(collisionData);
  }

  // Announce collision to components
  collisionData.source->gameObject.OnCollision(collisionData);

  // Switch reference
  swap(collisionData.source, collisionData.other);

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

  bool bodyBStatic = bodyB->GetType() == RigidbodyType::Static;

  // Correct position
  float bodyADisplacement = bodyBStatic ? penetration
                                        : penetration * bodyB->GetMass() / (bodyA->GetMass() + bodyB->GetMass());
  float bodyBDisplacement = penetration - bodyADisplacement;

  if (bodyA->GetType() != RigidbodyType::Static)
    bodyA->gameObject.Translate(-collisionData.normal * bodyADisplacement);

  if (bodyBStatic == false)
    bodyB->gameObject.Translate(collisionData.normal * bodyBDisplacement);
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
        *collider, trajectoryRectangle, trajectoryAngle, sourceObject);

    // Check if it's got better distance
    if (colliderDistance < minDistance)
    {
      minDistance = colliderDistance;
      selectedColliders = vector{collider};
    }
  }

  return make_tuple(selectedColliders, minDistance, []() {});
}

float GetDistanceAlongTrajectory(Collider &collider, Rectangle trajectoryRectangle, float trajectoryAngle, GameObject &sourceObject)
{
  // Get line equations for trajectory bounds
  auto boundA = GetLineEquation(
      trajectoryRectangle.TopLeft(trajectoryAngle), trajectoryRectangle.TopRight(trajectoryAngle));
  auto boundB = GetLineEquation(
      trajectoryRectangle.BottomLeft(trajectoryAngle), trajectoryRectangle.BottomRight(trajectoryAngle));

  auto body = sourceObject.GetComponent<Rigidbody>();
  body->printLines.clear();
  body->printLines.push_back(make_pair(Vector2(-10, GetY(-10, boundA)), Vector2(10, GetY(10, boundA))));
  body->printLines.push_back(make_pair(Vector2(-10, GetY(-10, boundB)), Vector2(10, GetY(10, boundB))));

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
    auto vertexAPosition = PointPositionForLines(*vertexA, boundA, boundB);
    auto vertexBPosition = PointPositionForLines(*vertexB, boundA, boundB);

    // If both vertices are inside, the face is already good
    if (vertexAPosition == PointPosition::Inside && vertexBPosition == PointPosition::Inside)
    {
      trimmedFacesVertices.push_back(*vertexA);
      trimmedFacesVertices.push_back(*vertexB);
      continue;
    }

    // Otherwise, need to find line equation of this face
    auto faceEquation = GetLineEquation(*vertexA, *vertexB);

    // If both are outside
    if (vertexAPosition != PointPosition::Inside && vertexBPosition != PointPosition::Inside)
    {
      // Ensure they are on opposite sides
      if (vertexAPosition == vertexBPosition)
        continue;

      // Impossible for them to be parallel, so no problem to search for intersection
      trimmedFacesVertices.push_back(GetIntersection(boundA, faceEquation));
      trimmedFacesVertices.push_back(GetIntersection(boundB, faceEquation));
      continue;
    }

    // If not, need to know which bound is the smaller
    bool boundASmaller = GetY(0, boundA) < GetY(0, boundB);

    // And which vertex is outside
    auto insideVertex = vertexAPosition == PointPosition::Inside ? *vertexA : *vertexB;

    // If the face crosses the smaller bound
    if (vertexAPosition == PointPosition::Smaller || vertexBPosition == PointPosition::Smaller)
    {
      // Impossible for them to be parallel, so no problem to search for intersection
      trimmedFacesVertices.push_back(GetIntersection(boundASmaller ? boundA : boundB, faceEquation));
      trimmedFacesVertices.push_back(insideVertex);

      continue;
    }

    // Otherwise the face crosses the bigger bound
    // Impossible for them to be parallel, so no problem to search for intersection
    trimmedFacesVertices.push_back(GetIntersection(boundASmaller ? boundB : boundA, faceEquation));
    trimmedFacesVertices.push_back(insideVertex);
  }

  // By the end of the loop, we should have all faces which are contained in the trajectory rectangle

  // Get the trajectory displacement vector, normalized
  auto displacementDirection = Vector2::Angled(trajectoryAngle);

  // Will store the minimum distance along the displacement
  float minDistance = numeric_limits<float>::max();

  // For each inside vertex
  body->intersectionPoints.clear();

  for (auto vertex : trimmedFacesVertices)
  {
    // if (Vector2::Dot(vertex, displacementDirection) < minDistance) {
    body->printIntersectionPoint = true;
    if (find(body->intersectionPoints.begin(), body->intersectionPoints.end(), vertex) == body->intersectionPoints.end())
      body->intersectionPoints.push_back(vertex);
    // }
    minDistance = min(minDistance, Vector2::Dot(vertex, displacementDirection));
  }

  return minDistance;
}

pair<float, float> GetLineEquation(Vector2 point1, Vector2 point2)
{
  // Prevent awkward stuff
  if (point2.x - point1.x == 0)
  {
    point1.x += 0.01;
  }

  float m = (point2.y - point1.y) / (point2.x - point1.x);

  return make_pair(m, point1.y - point1.x * m);
}

void PhysicsSystem::UnregisterColliders(int objectId)
{
  dynamicColliderStructure.erase(objectId);
  staticColliderStructure.erase(objectId);
}
