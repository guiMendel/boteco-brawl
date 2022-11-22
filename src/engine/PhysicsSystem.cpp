#include "Collider.h"
#include "PhysicsSystem.h"
#include "GameState.h"

using namespace std;
using namespace SatCollision;

void ApplyImpulse(CollisionData collisionData);

// The min value of penetration that can be considered a non collision entry contact
const float minStayPenetration{0.0001};

// Initial gravity
const Vector2 PhysicsSystem::initialGravity{0, 0};

PhysicsSystem::PhysicsSystem(GameState &gameState) : gameState(gameState) {}

void PhysicsSystem::PhysicsUpdate([[maybe_unused]] float deltaTime)
{
  ResolveCollisions();
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

      // If distance is negative, it implies collision
      if (distance < 0)
      {
        // If one of the colliders is a trigger, simply announce the trigger and carry on
        if (collider1->isTrigger || collider2->isTrigger)
        {
          collider1->gameObject.OnTriggerCollision(collider2->gameObject);
          collider2->gameObject.OnTriggerCollision(collider1->gameObject);

          continue;
        }

        // Populate collision data
        collisionData.source = collider1->rigidbodyWeak.lock();
        collisionData.other = collider2->rigidbodyWeak.lock();
        collisionData.normal = normal;
        collisionData.penetration = abs(distance);

        return true;
      }
    }
  }

  return false;
}

void PhysicsSystem::ResolveCollisions()
{
  // Get validated colliders
  auto validatedColliderStructure = ValidateAllColliders();

  // Will hold any collision data
  CollisionData collisionData;

  // For each object
  auto objectEntryIterator = validatedColliderStructure.begin();
  while (objectEntryIterator != validatedColliderStructure.end())
  {
    // Test, for each OTHER object in the list (excluding the ones before this one)
    decltype(objectEntryIterator) otherObjectEntryIterator{objectEntryIterator};
    otherObjectEntryIterator++;
    while (otherObjectEntryIterator != validatedColliderStructure.end())
    {
      // Check if they are colliding
      if (CheckForCollision(objectEntryIterator->second, otherObjectEntryIterator->second, collisionData))
        // Resolve collision (apply impulses)
        ResolveCollision(collisionData);

      otherObjectEntryIterator++;
    }

    objectEntryIterator++;
  }
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

  // If it has a rigidbody with auto mass on, derive its new mass
  auto rigidbody = gameState.GetObject(objectId)->GetComponent<Rigidbody>();

  if (rigidbody != nullptr && rigidbody->UsingAutoMass())
    rigidbody->DeriveMassFromColliders();
}

// Source https://youtu.be/1L2g4ZqmFLQ and https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics6collisionresponse/
void PhysicsSystem::ResolveCollision(CollisionData collisionData)
{
  // If this collision was already dealt with this frame, ignore it
  if (collisionData.source->IsCollidingWith(*collisionData.other))
    return;

  // ApplyImpulse(collisionData);

  // Check if is entering collision
  if (collisionData.source->WasCollidingWith(*collisionData.other) == false ||
      collisionData.penetration < minStayPenetration)
  {
    cout << "Penetration was " << collisionData.penetration << endl;

    // Apply impulse
    ApplyImpulse(collisionData);

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
  cout << "ApplyImpulse" << endl;

  // Ease of access
  auto bodyA = collisionData.source;
  auto bodyB = collisionData.other;

  // Elasticity of collision
  float elasticity = min(bodyA->elasticity, bodyB->elasticity);

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

  if (bodyA->type != RigidbodyType::Static)
    bodyA->gameObject.Translate(collisionData.normal * bodyADisplacement);

  if (bodyBStatic == false)
    bodyB->gameObject.Translate(collisionData.normal * bodyBDisplacement);
}