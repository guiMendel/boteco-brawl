#include "Game.h"
#include "Collider.h"
#include "PhysicsSystem.h"
#include "GameState.h"
#include <functional>
#include <tuple>

using namespace std;
using namespace Collision;

// Initial gravity
const Vector2 PhysicsSystem::initialGravity{0, 14};

// Min velocity before friction simply cuts it to 0
const float maxFrictionCutSpeed{0.001f};

// How many units to displace the raycast particle in each iteration
const float raycastGranularity{0.15f};

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
float GetDistanceAlongTrajectory(Collider &collider, Rigidbody &sourceBody);

void ApplyImpulse(CollisionData collisionData);

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
        // Try to find a body in one of the 2
        shared_ptr<Rigidbody> body;

        if ((body = collider1->rigidbodyWeak.lock()) != nullptr)
          ResolveTriggerCollision(*body, *collider2);
        else if ((body = collider2->rigidbodyWeak.lock()) != nullptr)
          ResolveTriggerCollision(*body, *collider1);

        // If no body was found, no trigger

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

  auto body = collidersEntryIterator->second.at(0)->RequireRigidbody();

  // Test, for each OTHER dynamic object in the list (excluding the ones before this one)
  auto otherCollidersEntryIterator{collidersEntryIterator};

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

  // cout << "Using continuous detection for " << objectBody->gameObject.GetName() << endl;

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
        FindTrajectoryIntersection(otherCollidersEntryIterator->second, *objectBody);

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
        FindTrajectoryIntersection(staticEntry.second, *objectBody);

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

  // cout << "Detected between frames collision with " << collisionTargets.at(0)->gameObject.GetName() << endl;

  // Make it relative to the object's position
  collisionDistance -= Vector2::Dot(objectBody->lastPosition, Vector2::Angled(trajectoryAngle));

  // Move the body to where collision happened
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
  // cout << "Resolving collision between " << collisionData.source->gameObject.GetName() << " and " << collisionData.other->gameObject.GetName() << endl;

  // If this collision was already dealt with this frame, ignore it
  if (collisionData.source->RequireRigidbody()->IsCollidingWith(collisionData.other->gameObject))
    return;

  ApplyImpulse(collisionData);

  // Check if is entering collision
  if (collisionData.source->RequireRigidbody()->WasCollidingWith(collisionData.other->gameObject) == false)
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

void PhysicsSystem::ResolveTriggerCollision(Rigidbody &body, Collider &collider)
{
  // If this collision was already dealt with this frame, ignore it
  if (body.IsCollidingWith(collider.gameObject))
    return;

  body.gameObject.OnTriggerCollision(collider.gameObject);
  collider.gameObject.OnTriggerCollision(body.gameObject);

  // Check if is entering collision
  if (body.WasCollidingWith(collider.gameObject) == false)
  {
    body.gameObject.OnTriggerCollisionEnter(collider.gameObject);
    collider.gameObject.OnTriggerCollisionEnter(body.gameObject);
  }
}

void ApplyImpulse(CollisionData collisionData)
{
  // Ease of access
  auto bodyA = collisionData.source->RequireRigidbody();
  auto bodyB = collisionData.other->RequireRigidbody();

  // Friction to apply
  float frictionModifier = min(bodyA->friction, bodyB->friction);

  // Apply it
  bodyA->velocity = PhysicsSystem::ApplyFriction(bodyA->velocity, frictionModifier);
  bodyB->velocity = PhysicsSystem::ApplyFriction(bodyB->velocity, frictionModifier);

  // Elasticity of collision
  float elasticity = (bodyA->elasticity + bodyB->elasticity) / 2.0f;

  // Relative velocity
  Vector2 relativeVelocity = bodyA->velocity - bodyB->velocity;

  // Calculate impulse vector along the normal
  float impulseMagnitude = -(1 + elasticity) * Vector2::Dot(relativeVelocity, collisionData.normal) /
                           (bodyA->GetInverseMass() + bodyB->GetInverseMass());

  // cout << "Impulse: " << impulseMagnitude << endl;

  Vector2 impulse = collisionData.normal * impulseMagnitude;

  // Apply impulse
  bodyA->ApplyImpulse(impulse);
  bodyB->ApplyImpulse(-impulse);

  // Increase penetration a little bit to ensure objects are no longer colliding afterwards
  float penetration = collisionData.penetration;

  bool bodyBStatic = bodyB->GetType() == RigidbodyType::Static;

  // Rectify position
  float bodyADisplacement = bodyBStatic ? penetration
                                        : penetration * bodyB->GetMass() / (bodyA->GetMass() + bodyB->GetMass());
  float bodyBDisplacement = penetration - bodyADisplacement;

  float displacementDirection = impulseMagnitude < 0 ? -1 : 1;

  bodyA->gameObject.Translate(collisionData.normal * bodyADisplacement * displacementDirection);

  // cout << "Displacement: " << bodyADisplacement << endl;
  // cout << "Normal: " << (string)collisionData.normal << endl;

  if (bodyBStatic == false)
    bodyB->gameObject.Translate(-collisionData.normal * bodyBDisplacement * displacementDirection);

  // cout << "Distance before: " << penetration << ", after: " << FindMinDistance(bodyA->GetColliders()[0]->GetBox(), bodyB->GetColliders()[0]->GetBox(), bodyA->gameObject.GetRotation(), bodyB->gameObject.GetRotation()).first << endl;

  // cout << "Normal before: " << (string)collisionData.normal << ", after: " << (string)FindMinDistance(bodyA->GetColliders()[0]->GetBox(), bodyB->GetColliders()[0]->GetBox(), bodyA->gameObject.GetRotation(), bodyB->gameObject.GetRotation()).second << endl;
}

auto PhysicsSystem::FindTrajectoryIntersection(ValidatedColliders colliders, Rigidbody &sourceBody)
    -> tuple<ValidatedColliders, float, function<void()>>
{
  auto [trajectoryRectangle, trajectoryAngle] = sourceBody.GetFrameTrajectory();

  // Get the associated rigidbody to the other body
  auto otherBody = colliders.at(0)->gameObject.GetComponent<Rigidbody>();

  // Check if it's continuous
  if (otherBody->ShouldUseContinuousDetection())
  {
    // Find out if there is intersection between the two trajectories first
    auto [intersectedColliders, intersectionDistance, intersectionDependencyCallback] =
        FindTrajectoryIntersectionDouble(*otherBody, sourceBody);

    // If an intersection is found, use it!
    if (intersectedColliders.empty() == false)
    {
      // cout << "Trajectories intersecting!" << endl;
      return make_tuple(intersectedColliders, intersectionDistance, intersectionDependencyCallback);
    }
  }

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
      ResolveTriggerCollision(sourceBody, *collider);
      continue;
    }

    // Get the distance of this collider along the trajectory
    float colliderDistance = GetDistanceAlongTrajectory(*collider, sourceBody);

    // Check if it's got better distance
    if (colliderDistance < minDistance)
    {
      minDistance = colliderDistance;
      selectedColliders = vector{collider};
    }
  }

  return make_tuple(selectedColliders, minDistance, []() {});
}

auto PhysicsSystem::FindTrajectoryIntersectionDouble(Rigidbody &otherBody, Rigidbody &sourceBody)
    -> tuple<ValidatedColliders, float, function<void()>>
{
  // Get trajectories
  auto [sourceRect, sourceAngle] = sourceBody.GetFrameTrajectory();
  auto [otherRect, otherAngle] = otherBody.GetFrameTrajectory();

  // If they're the same angle, ignore it
  if (otherAngle == sourceAngle)
    return make_tuple(vector<shared_ptr<Collider>>(), 0, []() {});

  // Check for intersection
  auto [distance, normal] = FindMinDistance(sourceRect, otherRect, sourceAngle, otherAngle);

  // If distance is positive, there is no intersection
  if (distance >= 0)
    return make_tuple(vector<shared_ptr<Collider>>(), 0, []() {});

  float sourceDistance;
  function<void()> otherDependency;
  float unstructuredOtherAngle = otherAngle;

  // If angles are opposite, take middle point
  if (sourceAngle == fmod(otherAngle + M_PI, 2 * M_PI))
  {
    float bodiesDistance = (otherBody.lastPosition - sourceBody.lastPosition).Magnitude();

    sourceDistance = (bodiesDistance - sourceBody.smallestDimension) / 2;

    otherDependency = [otherBody, unstructuredOtherAngle, bodiesDistance]()
    {
      float otherDistance = (bodiesDistance - otherBody.smallestDimension) / 2;

      otherBody.gameObject.SetPosition(otherBody.lastPosition + Vector2::Angled(unstructuredOtherAngle, otherDistance));
    };
  }

  // Otherwise, take intersection point between the lines
  else
  {
    // Find where these trajectories meet
    auto sourceLine = GetLineEquation(sourceBody.lastPosition, sourceBody.gameObject.GetPosition());
    auto otherLine = GetLineEquation(otherBody.lastPosition, otherBody.gameObject.GetPosition());

    auto intersectionPoint = GetIntersection(sourceLine, otherLine);

    sourceDistance = (intersectionPoint - sourceBody.lastPosition).Magnitude() - sourceBody.smallestDimension / 2;

    // Dependency of other object: if this collision is selected, it needs to be translated too!
    otherDependency = [otherBody, unstructuredOtherAngle, intersectionPoint]()
    {
      float otherDistance = (intersectionPoint - otherBody.lastPosition).Magnitude() - otherBody.smallestDimension / 2;

      otherBody.gameObject.SetPosition(otherBody.lastPosition + Vector2::Angled(unstructuredOtherAngle, otherDistance));
    };
  }

  return make_tuple(otherBody.GetColliders(), sourceDistance, otherDependency);
}

float GetDistanceAlongTrajectory(Collider &collider, Rigidbody &sourceBody)
{
  auto [trajectoryRectangle, trajectoryAngle] = sourceBody.GetFrameTrajectory();

  // Get line equations for trajectory bounds
  auto boundA = GetLineEquation(
      trajectoryRectangle.TopLeft(trajectoryAngle), trajectoryRectangle.TopRight(trajectoryAngle));
  auto boundB = GetLineEquation(
      trajectoryRectangle.BottomLeft(trajectoryAngle), trajectoryRectangle.BottomRight(trajectoryAngle));

  sourceBody.printLines.clear();
  sourceBody.printLines.push_back(make_pair(Vector2(-10, GetY(-10, boundA)), Vector2(10, GetY(10, boundA))));
  sourceBody.printLines.push_back(make_pair(Vector2(-10, GetY(-10, boundB)), Vector2(10, GetY(10, boundB))));

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
  sourceBody.intersectionPoints.clear();

  for (auto vertex : trimmedFacesVertices)
  {
    // if (Vector2::Dot(vertex, displacementDirection) < minDistance) {
    sourceBody.printIntersectionPoint = true;
    if (find(sourceBody.intersectionPoints.begin(), sourceBody.intersectionPoints.end(), vertex) == sourceBody.intersectionPoints.end())
      sourceBody.intersectionPoints.push_back(vertex);
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

Vector2 PhysicsSystem::ApplyFriction(Vector2 velocity, float friction)
{
  if (!velocity)
    return velocity;

  float frictionModifier = 1 - friction;

  // Find square of speed
  float sqrSpeed = velocity.SqrMagnitude();

  // Get this constant
  static const float sqrMaxCutSpeed{maxFrictionCutSpeed * maxFrictionCutSpeed};

  // Check if speed is relevant enough for us to still bother with it
  if (sqrSpeed <= sqrMaxCutSpeed)
    return Vector2::Zero();

  // Get the speed proportion factor
  float speedProportion = sqrSpeed > 1 ? pow(sqrSpeed, 0.25f) : 1;

  // Get delta time
  float deltaTime = Game::GetInstance().GetPhysicsDeltaTime();

  // Make friction proportional to delta time and speed
  float proportionalFriction = min(
      pow(pow(frictionModifier, speedProportion), deltaTime), 1.0f);

  // cout << "Friction: " << proportionalFriction << ". Old: " << velocity.Magnitude() << ", New: " << (velocity * proportionalFriction).Magnitude() << endl;

  return velocity * proportionalFriction;
}

bool PhysicsSystem::Raycast(Vector2 origin, float angle, float maxDistance, CollisionFilter filter)
{
  RaycastCollisionData discardedData;
  return Raycast(origin, angle, maxDistance, discardedData, filter);
}

bool PhysicsSystem::Raycast(Vector2 origin, float angle, float maxDistance, RaycastCollisionData &data, CollisionFilter filter)
{
  // How much the particle has already been displaced
  float displacement{0};

  while (displacement < maxDistance)
  {
    // Displace it
    displacement = min(displacement + raycastGranularity, maxDistance);

    // Get it's new position
    Vector2 particle = Vector2::Angled(angle, displacement) + origin;

    // Check for collision
    if (gameState.physicsSystem.DetectCollisions(particle, data, filter))
    {
      // Register distance
      data.elapsedDistance = displacement;

      return true;
    }
  }

  return false;
}

bool PhysicsSystem::DetectCollisions(Vector2 particle, RaycastCollisionData &data, CollisionFilter filter)
{
  // For a given collider structure, performs the check
  auto CheckForStructure = [&](std::unordered_map<int, PhysicsSystem::WeakColliders> structure)
  {
    for (auto [bodyId, bodyColliders] : ValidateAllColliders(structure))
    {
      // Skip filtered bodies
      if (filter.ignoredObjects.count(bodyId) > 0)
        continue;

      // For each collider
      for (auto collider : bodyColliders)
      {
        // Check if particle is far enough that we don't need to bother
        float sqrParticleDistance = Vector2::SqrDistance(collider->GetBox().Center(), particle);
        float maxDimension = collider->GetMaxVertexDistance();

        if (sqrParticleDistance > maxDimension * maxDimension)
          break;

        // Detect collision
        if (DetectIntersection(collider->GetBox(), particle, collider->gameObject.GetRotation()))
        {
          data.other = collider;

          return true;
        }
      }
    }

    return false;
  };

  return CheckForStructure(dynamicColliderStructure) || CheckForStructure(staticColliderStructure);
}
