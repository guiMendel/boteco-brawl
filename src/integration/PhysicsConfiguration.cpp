#include "PhysicsSystem.h"
#include "PhysicsLayerHandler.h"

// Initial gravity
const Vector2 PhysicsSystem::initialGravity{0, 14};

void PhysicsLayerHandler::InitializeCollisionMatrix()
{
  // Characters don't collide (normally) with each other
  Disable(PhysicsLayer::Character, PhysicsLayer::Character);

  // But their slide boxes do, and only with each other
  DisableAll(PhysicsLayer::CharacterSlideBox);
  Enable(PhysicsLayer::CharacterSlideBox, PhysicsLayer::CharacterSlideBox);
}
