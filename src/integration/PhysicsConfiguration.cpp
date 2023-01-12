#include "PhysicsSystem.h"
#include "PhysicsLayerHandler.h"

// Initial gravity
const Vector2 PhysicsSystem::initialGravity{0, 14};

void PhysicsLayerHandler::InitializeCollisionMatrix()
{
  // Characters don't collide (normally) with each other
  Disable(PhysicsLayer::Character, PhysicsLayer::Character);

  // But their repel boxes do collide only with characters
  DisableAll(PhysicsLayer::CharacterRepelBox);
  Enable(PhysicsLayer::CharacterRepelBox, PhysicsLayer::Character);

  // Platform detectors only collide with platforms
  DisableAll(PhysicsLayer::CharacterPlatformDrop);
  Enable(PhysicsLayer::CharacterPlatformDrop, PhysicsLayer::Platform);

  // Hitboxes can only hit characters
  DisableAll(PhysicsLayer::Hitbox);
  Enable(PhysicsLayer::Hitbox, PhysicsLayer::Character);
}
