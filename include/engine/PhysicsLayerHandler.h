#ifndef __PHYSICS_LAYER_HANDLER__
#define __PHYSICS_LAYER_HANDLER__

#define PHYSICS_LAYER_COUNT int(PhysicsLayer::Default) + 1

#include "PhysicsLayer.h"

class PhysicsLayerHandler
{
public:
  PhysicsLayerHandler();

  ~PhysicsLayerHandler() {}

  // Responsible for initializing definitions of which layers should collide
  // By default, all layers collide
  void InitializeCollisionMatrix();

  // Disables collision between two layers
  void Disable(PhysicsLayer, PhysicsLayer);

  // Enables collision between two layers
  void Enable(PhysicsLayer, PhysicsLayer);

  // Disables collision between a layer and all other layers
  void DisableAll(PhysicsLayer);

  // Enables collision between a layer and all other layer
  void EnableAll(PhysicsLayer);

  // Checks whether the two given layers collide
  bool HaveCollision(PhysicsLayer, PhysicsLayer) const;

private:
  // Collision detection matrix
  bool collisionMatrix[PHYSICS_LAYER_COUNT][PHYSICS_LAYER_COUNT];
};

#endif