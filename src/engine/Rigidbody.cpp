#include "Collider.h"
#include "Rigidbody.h"

using namespace std;

Rigidbody::Rigidbody(GameObject &associatedObject, RigidbodyType type, float elasticity)
    : Component(associatedObject), type(type), elasticity(elasticity) {}

float Rigidbody::GetMass() const
{
  if (type == RigidbodyType::Static)
    return std::numeric_limits<float>::max();

  return mass;
}

float Rigidbody::GetInverseMass() const
{
  if (type == RigidbodyType::Static)
    return 0;

  return inverseMass;
}

void Rigidbody::PhysicsUpdate(float deltaTime)
{
  if (type == RigidbodyType::Dynamic)
    DynamicBodyUpdate(deltaTime);

  // Update collision sets
  oldCollidingBodies = collidingBodies;
  collidingBodies.clear();
}

void Rigidbody::DynamicBodyUpdate(float deltaTime)
{
  // Apply gravity
  velocity += gameState.physicsSystem.gravity * gravityScale * deltaTime;

  // cout << gameObject.GetName() << " velocity x: " << velocity.x << endl;

  // Move according to velocity
  gameObject.Translate(velocity * deltaTime);
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
}

// Gets the list of colliders associated with this body
vector<shared_ptr<Collider>> Rigidbody::GetColliders() const
{
  return gameState.physicsSystem.ValidateColliders(gameObject.id);
}

void Rigidbody::ApplyImpulse(Vector2 impulse)
{
  if (type == RigidbodyType::Static)
    return;

  cout << "Impulse: " << (string)impulse << ", final velocity: " << (string)(velocity + impulse * inverseMass) << endl;

  velocity += impulse * inverseMass;
}

bool Rigidbody::IsCollidingWith(Rigidbody &otherBody)
{
  return collidingBodies.count(otherBody.gameObject.id) > 0;
}

bool Rigidbody::WasCollidingWith(Rigidbody &otherBody)
{
  return oldCollidingBodies.count(otherBody.gameObject.id) > 0;
}

void Rigidbody::OnCollision(SatCollision::CollisionData collisionData)
{
  // Add to collision set
  collidingBodies.insert(collisionData.other->gameObject.id);
}
