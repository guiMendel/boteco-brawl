#include "Rigidbody.h"
#include "PhysicsSystem.h"

Rigidbody::Rigidbody(GameObject &associatedObject, RigidbodyType type) : Component(associatedObject), type(type) {}

void Rigidbody::Update(float deltaTime)
{
  if (type == RigidbodyType::Static)
    return;

  // Apply gravity
  velocity += PhysicsSystem::gravity * deltaTime;

  // Move according to velocity
  gameObject.SetPosition(gameObject.GetPosition() + velocity * deltaTime);
}
