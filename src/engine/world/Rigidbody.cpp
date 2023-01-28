#include "Collider.h"
#include "Rigidbody.h"
#include "Game.h"
#include "Camera.h"
#include "Debug.h"

using namespace std;

const float Rigidbody::defaultAirFriction{0.0f};

Rigidbody::Rigidbody(WorldObject &associatedObject, RigidbodyType type, float elasticity, float friction)
    : Component(associatedObject), elasticity(elasticity), friction(friction), type(type), lastPosition(worldObject.GetPosition()) {}

float Rigidbody::GetMass() const
{
  if (IsStatic() || IsKinematic())
    return numeric_limits<float>::max();

  return mass;
}

float Rigidbody::GetInverseMass() const
{
  if (IsStatic() || IsKinematic())
    return 0;

  return inverseMass;
}

void Rigidbody::PhysicsUpdate(float deltaTime)
{
  if (IsDynamic())
    DynamicBodyUpdate(deltaTime);

  else if (IsKinematic())
    KinematicBodyUpdate(deltaTime);
}

void Rigidbody::DynamicBodyUpdate(float deltaTime)
{
  // Apply gravity
  velocity += GetState()->physicsSystem.gravity * gravityScale * deltaTime;

  // Apply air friction
  velocity = PhysicsSystem::ApplyFriction(velocity, airFriction, worldObject.GetTimeScale());

  // Update the last position variable
  lastPosition = worldObject.GetPosition();

  KinematicBodyUpdate(deltaTime);
}

void Rigidbody::KinematicBodyUpdate(float deltaTime)
{
  // Move according to velocity
  worldObject.Translate(velocity * deltaTime);
}

void Rigidbody::UseAutoMass(bool value)
{
  useAutoMass = value;

  // Calculate it
  DeriveMassFromColliders();
}

void Rigidbody::InternalSetMass(float newMass)
{
  mass = newMass;
  inverseMass = 1.0f / newMass;
}

void Rigidbody::SetMass(float newMass)
{
  if (useAutoMass)
  {
    cout << "WARNING: tried setting mass when useAutoMass is set to true" << endl;
    return;
  }

  InternalSetMass(newMass);
}

void Rigidbody::DeriveMassFromColliders()
{
  float totalMass = 0;

  for (auto collider : GetColliders())
  {
    totalMass += collider->GetMass();
  }

  InternalSetMass(totalMass);

  cout << worldObject << " calculated mass is " << totalMass << endl;
}

// Gets the list of colliders associated with this body
vector<shared_ptr<Collider>> Rigidbody::GetColliders() const
{
  auto &physicsSystem = GetState()->physicsSystem;

  PhysicsSystem::WeakColliders weakColliders;
  if (IsStatic())
    weakColliders = physicsSystem.staticColliderStructure[worldObject.id];
  else if (IsKinematic())
    weakColliders = physicsSystem.kinematicColliderStructure[worldObject.id];
  else
    weakColliders = physicsSystem.dynamicColliderStructure[worldObject.id];

  return physicsSystem.ValidateColliders(worldObject.id, weakColliders);
}

void Rigidbody::ApplyImpulse(Vector2 impulse)
{
  if (IsStatic() || IsKinematic())
    return;

  velocity += impulse * inverseMass;
}

bool Rigidbody::ShouldUseContinuousDetection() const
{
  // Cut short if disabled
  if (continuousCollisions == false)
    return false;

  // Check that it's moved at least as much as it's smallest dimension
  auto sqrDistance = (worldObject.GetPosition() - lastPosition).SqrMagnitude();

  float dimension = GetMinDimension();

  // Only use continuous if it's moved more than this
  return sqrDistance > dimension * dimension;
}

Vector2 Rigidbody::GetFrameTrajectory()
{
  return worldObject.GetPosition() - lastPosition;
}

void Rigidbody::Render() {}

RigidbodyType Rigidbody::GetType() const { return type; }

void Rigidbody::SetType(RigidbodyType newType)
{
  if (type == newType)
    return;

  type = newType;
  GetState()->physicsSystem.UnregisterColliders(worldObject.id);

  // Re-register colliders
  for (auto collider : GetColliders())
    GetState()->physicsSystem.RegisterCollider(collider, worldObject.id);
}

bool Rigidbody::Raycast(float angle, float maxDistance, RaycastData &data)
{
  // Create a filter of this own object
  CollisionFilter filter;
  filter.ignoredObjects.insert(worldObject.id);

  return GetState()->physicsSystem.Raycast(worldObject.GetPosition(), angle, maxDistance, data, filter);
}

bool Rigidbody::Raycast(float angle, float maxDistance)
{
  RaycastData discardedData;
  return Raycast(angle, maxDistance, discardedData);
}

bool Rigidbody::ColliderCast(float angle, float maxDistance, ColliderCastData &data, float scaleColliders)
{
  // Create a filter of this own object
  CollisionFilter filter;
  filter.ignoredObjects.insert(worldObject.id);

  return GetState()->physicsSystem.ColliderCast(GetColliders(), worldObject.GetPosition(), angle, maxDistance, data, filter, scaleColliders);
}

bool Rigidbody::ColliderCast(float angle, float maxDistance, float scaleColliders)
{
  ColliderCastData discardedData;
  return ColliderCast(angle, maxDistance, discardedData, scaleColliders);
}

bool Rigidbody::IsStatic() const { return type == RigidbodyType::Static; }

bool Rigidbody::IsKinematic() const { return type == RigidbodyType::Kinematic; }

bool Rigidbody::IsDynamic() const { return type == RigidbodyType::Dynamic; }

float Rigidbody::GetMinDimension() const
{
  float smallestDimension = numeric_limits<float>::max();

  // For each collider
  for (auto collider : GetColliders())
    // Compare
    smallestDimension = min(collider->DeriveShape()->GetMinDimension(), smallestDimension);

  return smallestDimension;
}
