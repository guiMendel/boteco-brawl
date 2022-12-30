#include "PhysicsLayerHandler.h"

void PhysicsLayerHandler::InitializeCollisionMatrix()
{
  // Characters don't collide (normally) with each other
  Disable(PhysicsLayer::Character, PhysicsLayer::Character);
}
