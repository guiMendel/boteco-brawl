#include "Game.h"
#include "Collider.h"
#include "PlatformEffector.h"
#include "PhysicsSystem.h"
#include "GameScene.h"
#include <functional>
#include <tuple>
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

// Min velocity before friction simply cuts it to 0
const float maxFrictionCutSpeed{0.001f};

// How many units to displace the raycast particle in each iteration
const float raycastGranularity{0.15f};

void ApplyImpulse(Collision::Data collisionData);

// Given that the 2 colliders collided, checks if a platform effector allows this collision through
bool PlatformEffectorCheck(Collider &collider1, Collider &collider2);

PhysicsSystem::PhysicsSystem(GameScene &gameScene) : gameScene(gameScene) {}

void PhysicsSystem::PhysicsUpdate(float)
{
  HandleCollisions();
}

bool PlatformEffectorCheck(Collider &collider1, Collider &collider2)
{
  // Checks given a body and a collider
  auto CheckFor = [](shared_ptr<Rigidbody> body, Collider &collider)
  {
    if (body == nullptr)
      return false;

    auto platform = collider.worldObject.GetComponent<PlatformEffector>();

    if (platform == nullptr)
      return false;

    return platform->AllowThrough(body);
  };

  // Both checks need to be performed (for PlatformEffector's internal state coherence), so we perform or on their already computed results
  bool check1 = CheckFor(collider1.rigidbodyWeak.lock(), collider2);
  bool check2 = CheckFor(collider2.rigidbodyWeak.lock(), collider1);

  return check1 || check2;
}

// Returns whether the two collider lists have some pair of colliders which are intersecting
// If there is, also populates the collision data struct
bool PhysicsSystem::CheckForCollision(
    vector<shared_ptr<Collider>> colliders1,
    vector<shared_ptr<Collider>> colliders2,
    Collision::Data &collisionData,
    Vector2 displaceColliders1,
    float scaleColliders1)
{
  for (auto collider1 : colliders1)
  {
    // Verify if enabled
    if (collider1->IsEnabled() == false)
      continue;

    for (auto collider2 : colliders2)
    {
      // Verify if enabled
      if (collider2->IsEnabled() == false)
        continue;

      // Verify collision matrix
      if (layerHandler.HaveCollision(collider1->worldObject, collider2->worldObject) == false)
        continue;

      // Apply displacement & scale
      auto shape1 = collider1->DeriveShape();
      shape1->Displace(displaceColliders1);
      shape1->Scale({scaleColliders1, scaleColliders1});

      auto [distance, normal] = Collision::FindMinDistance(shape1, collider2->DeriveShape());

      // If distance is positive, or a PlatformEffector allows collision through, there is no collision
      if (distance >= 0 || PlatformEffectorCheck(*collider1, *collider2))
        continue;

      // Populate collision data
      collisionData.weakSource = collider1;
      collisionData.weakOther = collider2;
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
  auto kinematicColliders = ValidateAllColliders(kinematicColliderStructure);

  // Merge static with kinematic
  auto nonDynamicColliders = ValidateAllColliders(staticColliderStructure);
  nonDynamicColliders.insert(nonDynamicColliders.end(), kinematicColliders.begin(), kinematicColliders.end());

  // Check collisions for each dynamic object
  for (
      auto collidersIterator = dynamicColliders.begin();
      collidersIterator != dynamicColliders.end();
      collidersIterator++)
  {
    Assert(collidersIterator->empty() == false, "Collider entry was unexpectedly empty");

    // Check for continuous detection
    auto objectBody = collidersIterator->at(0)->RequireRigidbody();

    // Check for disable
    if (objectBody->IsEnabled() == false)
      continue;

    if (objectBody->ShouldUseContinuousDetection())
      DetectObjectBetweenFramesCollision(collidersIterator);

    else
      DetectObjectCollisions(collidersIterator, dynamicColliders.end(), nonDynamicColliders);
  }

  // Get triggers
  auto triggerColliders = ValidateAllColliders(weakTriggerColliders);

  // Check trigger collisions for each trigger
  for (
      auto triggerIterator = triggerColliders.begin();
      triggerIterator != triggerColliders.end();
      triggerIterator++)
  {
    // Get trigger data
    auto triggerCollider = *triggerIterator;
    auto triggerBody = triggerCollider->rigidbodyWeak.lock();
    bool isStatic = triggerBody == nullptr || triggerBody->IsStatic();

    // Store collision data
    static Collision::Collision::Data collisionData;

    // Check against all dynamic objects
    for (auto colliders : dynamicColliders)
    {
      if (WorldObject::SameLineage(
              *gameScene.GetObject(triggerCollider->GetOwnerId()),
              *gameScene.GetObject(colliders.at(0)->GetOwnerId())))
        continue;

      if (CheckForCollision(colliders, {triggerCollider}, collisionData))
        ResolveTriggerCollision(collisionData.weakSource.lock(), collisionData.weakOther.lock());
    }

    // Check against other statics only if not static
    auto nonDynamicTargets = isStatic ? kinematicColliders : nonDynamicColliders;
    for (auto colliders : nonDynamicTargets)
    {
      if (WorldObject::SameLineage(
              *gameScene.GetObject(triggerCollider->GetOwnerId()),
              *gameScene.GetObject(colliders.at(0)->GetOwnerId())))
        continue;

      if (CheckForCollision(colliders, {triggerCollider}, collisionData))
        ResolveTriggerCollision(collisionData.weakSource.lock(), collisionData.weakOther.lock());
    }

    // Check against each other trigger collider
    for (
        auto otherTriggerIterator = triggerIterator;
        otherTriggerIterator != triggerColliders.end();
        otherTriggerIterator++)
    {
      // Get it's data
      auto otherTriggerCollider = *otherTriggerIterator;
      auto otherTriggerBody = otherTriggerCollider->rigidbodyWeak.lock();

      if (WorldObject::SameLineage(
              *gameScene.GetObject(triggerCollider->GetOwnerId()),
              *gameScene.GetObject(otherTriggerCollider->GetOwnerId())))
        continue;

      // Ignore it if both are static
      if (isStatic && (otherTriggerBody == nullptr || otherTriggerBody->IsStatic()))
        continue;

      if (CheckForCollision({otherTriggerCollider}, {triggerCollider}, collisionData))
        ResolveTriggerCollision(collisionData.weakSource.lock(), collisionData.weakOther.lock());
    }
  }
}

void PhysicsSystem::DetectObjectCollisions(
    vector<ValidatedColliders>::iterator collidersIterator,
    vector<ValidatedColliders>::iterator endIterator,
    vector<ValidatedColliders> &nonDynamicColliders)
{
  // Will hold any collision data
  static Collision::Data collisionData;

  // Body of this object
  auto body = collidersIterator->at(0)->RequireRigidbody();

  // Test, for each OTHER dynamic object in the list (excluding the ones before this one)
  auto otherCollidersIterator{collidersIterator};

  for (otherCollidersIterator++; otherCollidersIterator != endIterator; otherCollidersIterator++)
  {
    // Check if they are colliding
    if (CheckForCollision(*collidersIterator, *otherCollidersIterator, collisionData))
      // Resolve collision (apply impulses)
      ResolveCollision(collisionData);
  }

  // Test for all non dynamic objects
  for (auto otherColliders : nonDynamicColliders)
  {
    // cout << "Checking for object " << body->worldObject.GetName() << " with " << collidersIterator->second.size() << " colliders against " << otherColliders.at(0)->worldObject.GetName() << " with " << otherColliders.size() << " colliders" << endl;
    // Check if they are colliding
    // Other object's layer
    if (CheckForCollision(*collidersIterator, otherColliders, collisionData))
      // Resolve collision (apply impulses)
      ResolveCollision(collisionData);
  }
}

void PhysicsSystem::DetectObjectBetweenFramesCollision(vector<ValidatedColliders>::iterator collidersIterator)
{
  // Get object body
  auto objectBody = collidersIterator->at(0)->RequireRigidbody();

  cout << "Using continuous detection for " << objectBody->worldObject.GetName() << endl;

  // Get trajectory info
  auto trajectory = objectBody->GetFrameTrajectory();

  // Check if a collision should have happened
  ColliderCastData castData;

  bool collisionFound = ColliderCast(
      *collidersIterator,
      objectBody->lastPosition,
      trajectory.Angle(),
      trajectory.Magnitude(),
      castData);

  // Trigger any found triggers
  for (auto triggerData : castData.triggerCollisions)
  {
    LOCK(triggerData.weakSource, source);
    LOCK(triggerData.weakOther, other);
    ResolveTriggerCollision(source, other);
  }

  // If no collision, stop
  if (collisionFound == false)
    return;

  LOCK(castData.collision.weakOther, otherCollider);
  cout << "Detected between frames collision with " << otherCollider->worldObject.GetName() << endl;

  // Move the body to where collision happened
  objectBody->worldObject.SetPosition(
      objectBody->lastPosition + Vector2::Angled(trajectory.Angle(), castData.elapsedDistance));

  // Resolve the encountered collision
  ResolveCollision(castData.collision);
}

vector<shared_ptr<Collider>> PhysicsSystem::ValidateColliders(int id, WeakColliders &weakColliders)
{
  if (gameScene.GetObject(id) == nullptr)
    return {};

  // Will hold verified colliders
  vector<shared_ptr<Collider>> verifiedColliders;

  // Will recalculate rigidbody mass if a collider is removed
  bool colliderWasRemoved{false};

  // For each of it's colliders
  auto colliderIterator = weakColliders.begin();
  while (colliderIterator != weakColliders.end())
  {
    // Remove it if it's expired
    if (colliderIterator->expired())
    {
      colliderIterator = weakColliders.erase(colliderIterator);
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
    auto rigidbody = gameScene.GetObject(id)->GetComponent<Rigidbody>();

    if (rigidbody != nullptr)
    {
      if (rigidbody->UsingAutoMass())
        rigidbody->DeriveMassFromColliders();
    }
  }

  return verifiedColliders;
}

auto PhysicsSystem::ValidateAllColliders(unordered_map<int, weak_ptr<Collider>> &weakColliders)
    -> vector<shared_ptr<Collider>>
{
  vector<shared_ptr<Collider>> colliders;

  // For each object entry
  auto collidersEntryIterator = weakColliders.begin();
  while (collidersEntryIterator != weakColliders.end())
  {
    auto [objectId, weakCollider] = *collidersEntryIterator;

    // If it's empty, remove it from the map
    if (weakCollider.expired())
    {
      collidersEntryIterator = weakColliders.erase(collidersEntryIterator);
      continue;
    }

    colliders.push_back(weakCollider.lock());
    collidersEntryIterator++;
  }

  // Obtain a time-based seed
  unsigned seed = chrono::system_clock::now().time_since_epoch().count();

  // Shuffle colliders
  shuffle(colliders.begin(), colliders.end(), default_random_engine(seed));

  return colliders;
}

auto PhysicsSystem::ValidateAllColliders(unordered_map<int, WeakColliders> &weakColliders)
    -> vector<ValidatedColliders>
{
  vector<ValidatedColliders> verifiedCollidersStructure;

  // For each object entry
  auto collidersEntryIterator = weakColliders.begin();
  while (collidersEntryIterator != weakColliders.end())
  {
    int objectId = collidersEntryIterator->first;
    auto objectColliders = ValidateColliders(objectId, collidersEntryIterator->second);

    // If it's empty, remove it from the map
    if (objectColliders.empty())
    {
      collidersEntryIterator = weakColliders.erase(collidersEntryIterator);
      continue;
    }

    verifiedCollidersStructure.push_back(objectColliders);
    collidersEntryIterator++;
  }

  // Obtain a time-based seed
  unsigned seed = chrono::system_clock::now().time_since_epoch().count();

  // Shuffle colliders
  shuffle(verifiedCollidersStructure.begin(), verifiedCollidersStructure.end(), default_random_engine(seed));

  return verifiedCollidersStructure;
}

void PhysicsSystem::RegisterCollider(shared_ptr<Collider> collider, int objectId)
{
  if (!collider)
    return;

  // Register triggers
  if (collider->isTrigger)
  {
    weakTriggerColliders[collider->id] = collider;
    return;
  }

  // Get rigidbody if it exists
  auto rigidbody = collider->rigidbodyWeak.lock();

  // Check if it's static
  bool isStatic = rigidbody == nullptr || rigidbody->IsStatic();

  if (isStatic)
    staticColliderStructure[objectId].emplace_back(collider);
  else if (rigidbody->IsKinematic())
    kinematicColliderStructure[objectId].emplace_back(collider);
  else
    dynamicColliderStructure[objectId].emplace_back(collider);

  if (rigidbody != nullptr)
  {
    // If it has auto mass on, derive its new mass
    if (rigidbody->UsingAutoMass())
      rigidbody->DeriveMassFromColliders();
  }

  // cout << "After registration:" << endl;
  // for (auto [id, colliders] : dynamicColliderStructure)
  //   cout << "  Dynamic Object " << gameScene.GetObject(id)->GetName() << " has " << colliders.size() << ", first is expired: " << colliders.at(0).expired() << endl;

  // for (auto [id, colliders] : staticColliderStructure)
  //   cout << "  Static Object " << gameScene.GetObject(id)->GetName() << " has " << colliders.size() << endl;
  // cout << "Done." << endl;
}

// Source https://youtu.be/1L2g4ZqmFLQ and https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics6collisionresponse/
void PhysicsSystem::ResolveCollision(Collision::Data collisionData1)
{
  // cout << "Resolving collision between " << collisionData.source->worldObject.GetName() << " and " << collisionData.other->worldObject.GetName() << endl;
  auto collider1 = collisionData1.weakSource.lock();
  auto collider2 = collisionData1.weakOther.lock();

  if (collider1 == nullptr || collider2 == nullptr)
    return;

  // If this collision was already dealt with this frame, ignore it
  if (collider1->worldObject.CollisionDealtWith(collisionData1))
    return;

  // Build another collision data, and switch it's reference
  auto collisionData2{collisionData1};
  swap(collisionData2.weakSource, collisionData2.weakOther);

  // Get bodies
  auto body1 = collider1->rigidbodyWeak.lock();
  auto body2 = collider2->rigidbodyWeak.lock();

  // Forget a body if it's the same object as the collider
  if (body1 != nullptr && body1->worldObject == collider1->worldObject)
    body1 = nullptr;
  if (body2 != nullptr && body2->worldObject == collider2->worldObject)
    body2 = nullptr;

  // Resolve physics
  ApplyImpulse(collisionData1);

  // Check if is entering collision
  if (collider1->worldObject.CollisionDealtWithLastFrame(collisionData1) == false)
  {
    // Announce collision enter to components
    collider1->worldObject.OnCollisionEnter(collisionData1);
    collider2->worldObject.OnCollisionEnter(collisionData2);
    if (body1 != nullptr)
      body1->worldObject.OnCollisionEnter(collisionData1);
    if (body2 != nullptr)
      body2->worldObject.OnCollisionEnter(collisionData2);
  }

  // Announce collision enter to components
  collider1->worldObject.OnCollision(collisionData1);
  collider2->worldObject.OnCollision(collisionData2);
  if (body1 != nullptr)
    body1->worldObject.OnCollision(collisionData1);
  if (body2 != nullptr)
    body2->worldObject.OnCollision(collisionData2);
}

void PhysicsSystem::ResolveTriggerCollision(shared_ptr<Collider> collider1, shared_ptr<Collider> collider2)
{
  // Create structs
  TriggerCollisionData triggerData1{collider1, collider2}, triggerData2{collider2, collider1};

  // If this collision was already dealt with this frame, ignore it
  if (collider1->worldObject.TriggerCollisionDealtWith(triggerData1))
    return;

  // Get bodies
  auto body1 = collider1->rigidbodyWeak.lock();
  auto body2 = collider2->rigidbodyWeak.lock();

  // Forget a body if it's the same object as the collider
  if (body1 != nullptr && body1->worldObject == collider1->worldObject)
    body1 = nullptr;
  if (body2 != nullptr && body2->worldObject == collider2->worldObject)
    body2 = nullptr;

  // Check if is entering collision
  if (collider1->worldObject.TriggerCollisionDealtWithLastFrame(triggerData1) == false)
  {
    // Raise for involved objects
    collider1->worldObject.OnTriggerCollisionEnter(triggerData1);
    collider2->worldObject.OnTriggerCollisionEnter(triggerData2);
    if (body1 != nullptr)
      body1->worldObject.OnTriggerCollisionEnter(triggerData1);
    if (body2 != nullptr)
      body2->worldObject.OnTriggerCollisionEnter(triggerData2);
  }

  // Raise for involved objects
  collider1->worldObject.OnTriggerCollision(triggerData1);
  collider2->worldObject.OnTriggerCollision(triggerData2);
  if (body1 != nullptr)
    body1->worldObject.OnTriggerCollision(triggerData1);
  if (body2 != nullptr)
    body2->worldObject.OnTriggerCollision(triggerData2);
}

void ApplyImpulse(Collision::Data collisionData)
{
  // Ease of access
  auto bodyA = collisionData.weakSource.lock()->RequireRigidbody();
  auto bodyB = collisionData.weakOther.lock()->RequireRigidbody();

  // Friction to apply
  float frictionModifier = min(bodyA->friction, bodyB->friction);

  // Apply it
  bodyA->velocity = PhysicsSystem::ApplyFriction(bodyA->velocity, frictionModifier, bodyB->worldObject.GetTimeScale());
  bodyB->velocity = PhysicsSystem::ApplyFriction(bodyB->velocity, frictionModifier, bodyA->worldObject.GetTimeScale());

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

  bodyA->worldObject.Translate(collisionData.normal * bodyADisplacement * displacementDirection);

  // cout << "Displacement: " << bodyADisplacement << endl;
  // cout << "Normal: " << (string)collisionData.normal << endl;

  if (bodyBStatic == false)
    bodyB->worldObject.Translate(-collisionData.normal * bodyBDisplacement * displacementDirection);
}

void PhysicsSystem::UnregisterColliders(int objectId)
{
  dynamicColliderStructure.erase(objectId);
  kinematicColliderStructure.erase(objectId);
  staticColliderStructure.erase(objectId);
}

Vector2 PhysicsSystem::ApplyFriction(Vector2 velocity, float friction, float timeScale)
{
  if (!velocity || friction == 0)
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
  float deltaTime = Game::GetInstance().GetPhysicsDeltaTime() * timeScale;

  // Make friction proportional to delta time and speed
  float proportionalFriction = min(
      pow(pow(frictionModifier, speedProportion), deltaTime), 1.0f);

  // cout << "Friction: " << proportionalFriction << ". Old: " << velocity.Magnitude() << ", New: " << (velocity * proportionalFriction).Magnitude() << endl;

  return velocity * proportionalFriction;
}

bool PhysicsSystem::Raycast(Vector2 origin, float angle, float maxDistance, CollisionFilter filter)
{
  RaycastData discardedData;
  return Raycast(origin, angle, maxDistance, discardedData, filter);
}

bool PhysicsSystem::Raycast(Vector2 origin, float angle, float maxDistance, RaycastData &data, CollisionFilter filter)
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
    if (DetectRaycastCollisions(particle, data, filter))
    {
      // Register distance
      data.elapsedDistance = displacement;

      return true;
    }
  }

  return false;
}

bool PhysicsSystem::DetectRaycastCollisions(Vector2 particle, RaycastData &data, CollisionFilter filter)
{
  // For a given collider structure, performs the check
  auto CheckForStructure = [&](unordered_map<int, PhysicsSystem::WeakColliders> structure)
  {
    for (auto bodyColliders : ValidateAllColliders(structure))
    {
      auto bodyId = bodyColliders.at(0)->GetOwnerId();

      // Skip filtered bodies
      if (filter.ignoredObjects.count(bodyId) > 0)
        continue;

      // For each collider
      for (auto collider : bodyColliders)
      {
        // Check if particle is far enough that we don't need to bother
        float sqrParticleDistance = Vector2::SqrDistance(collider->DeriveShape()->center, particle);
        float maxEdgeCenterDistance = collider->DeriveShape()->GetMaxDimension() / 2;

        if (sqrParticleDistance > maxEdgeCenterDistance * maxEdgeCenterDistance)
          break;

        // Detect collision
        if (collider->DeriveShape()->Contains(particle))
        {
          data.other = collider;

          return true;
        }
      }
    }

    return false;
  };

  return CheckForStructure(dynamicColliderStructure) || CheckForStructure(kinematicColliderStructure) || CheckForStructure(staticColliderStructure);
}

bool PhysicsSystem::ColliderCast(vector<shared_ptr<Collider>> colliders, Vector2 origin, float angle, float maxDistance, CollisionFilter filter, float colliderSizeScale)
{
  ColliderCastData discardedData;
  return ColliderCast(colliders, origin, angle, maxDistance, discardedData, filter, colliderSizeScale);
}

bool PhysicsSystem::ColliderCast(vector<shared_ptr<Collider>> colliders, Vector2 origin, float angle, float maxDistance, ColliderCastData &data, CollisionFilter filter, float colliderSizeScale)
{
  if (colliders.size() == 0)
    return false;

  data.triggerCollisions.clear();

  // Vector to displace from colliders' current position to the origin parameter
  Vector2 originDisplacement = origin - colliders[0]->worldObject.GetPosition();

  // Get the min collider dimension
  float minColliderDimension{numeric_limits<float>::max()};
  for (auto collider : colliders)
    minColliderDimension = min(minColliderDimension, collider->DeriveShape()->GetMinDimension());

  // How much the colliders have already been displaced
  float displacement{0};

  while (displacement < maxDistance)
  {
    // Displace them
    displacement = min(displacement + minColliderDimension / 4.0f, maxDistance);

    // Check for collision
    if (DetectColliderCastCollisions(colliders, originDisplacement + Vector2::Angled(angle, displacement), data, filter, colliderSizeScale))
    {
      // Register distance
      data.elapsedDistance = displacement;

      return true;
    }
  }

  return false;
}

// Returns whether detected a collision between the given colliders and any bodies
bool PhysicsSystem::DetectColliderCastCollisions(vector<shared_ptr<Collider>> colliders, Vector2 displacement, ColliderCastData &data, CollisionFilter filter, float colliderSizeScale)
{
  // Keep track of our colliders' owner ids
  unordered_set<int> collidersIds;
  for (auto collider : colliders)
    collidersIds.insert(collider->GetOwnerId());

  // For a given collider structure, performs the check
  auto CheckForStructure = [&](unordered_map<int, PhysicsSystem::WeakColliders> &structure)
  {
    for (auto otherColliders : ValidateAllColliders(structure))
    {
      auto otherId = otherColliders.at(0)->GetOwnerId();

      // Skip filtered bodies & these colliders's owners
      if (filter.ignoredObjects.count(otherId) > 0 || collidersIds.count(otherId) > 0)
        continue;

      // Detect collisions between the given colliders
      if (CheckForCollision(colliders, otherColliders, data.collision, displacement, colliderSizeScale))
        return true;
    }

    return false;
  };

  // Detect triggers
  for (auto otherCollider : ValidateAllColliders(weakTriggerColliders))
  {
    auto otherId = otherCollider->GetOwnerId();

    // Skip filtered bodies & these colliders's owners
    if (filter.ignoredObjects.count(otherId) > 0 || collidersIds.count(otherId) > 0)
      continue;

    // Detect collisions between the given colliders
    Collision::Collision::Data triggerCollision;
    if (CheckForCollision(colliders, {otherCollider}, triggerCollision, displacement, colliderSizeScale))
    {
      TriggerCollisionData triggerData;
      triggerData.weakSource = triggerCollision.weakSource;
      triggerData.weakOther = triggerCollision.weakOther;
      data.triggerCollisions.push_back(triggerData);
    }
  }

  return CheckForStructure(dynamicColliderStructure) || CheckForStructure(kinematicColliderStructure) || CheckForStructure(staticColliderStructure);
}
