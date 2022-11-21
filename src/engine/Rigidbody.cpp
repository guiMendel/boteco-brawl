#include "Rigidbody.h"

using namespace std;

Rigidbody::Rigidbody(GameObject &associatedObject, RigidbodyType type, float elasticity)
    : Component(associatedObject), type(type), elasticity(elasticity) {}

void Rigidbody::Update(float deltaTime)
{
  if (type == RigidbodyType::Static)
    return;

  // Apply gravity
  velocity += gameState.physicsSystem.gravity * gravityScale * deltaTime;

  // Move according to velocity
  gameObject.SetPosition(gameObject.GetPosition() + velocity * deltaTime);
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
    totalMass += collider->GetMass();

  InternalSetMass(totalMass);
}

// Gets the list of colliders associated with this body
vector<shared_ptr<Collider>> Rigidbody::GetColliders() const
{
  return gameState.physicsSystem.ValidateColliders(gameObject.id);
}

void Rigidbody::ApplyImpulse(Vector2 impulse)
{
  velocity = velocity + impulse * inverseMass;
}
