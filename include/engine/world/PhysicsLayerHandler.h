#ifndef __PHYSICS_LAYER_HANDLER__
#define __PHYSICS_LAYER_HANDLER__

#define PHYSICS_LAYER_COUNT int(PhysicsLayer::None)

#include "PhysicsLayer.h"
#include <unordered_map>
#include <string>

class WorldObject;

class PhysicsLayerHandler
{
public:
  PhysicsLayerHandler();

  virtual ~PhysicsLayerHandler() {}

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
  bool HaveCollision(WorldObject &, WorldObject &) const;

private:
  // Prints the collision matrix table
  void PrintMatrix();

  // Collision detection matrix
  bool collisionMatrix[PHYSICS_LAYER_COUNT][PHYSICS_LAYER_COUNT];

  // Translation table from enum int to string
  std::unordered_map<PhysicsLayer, std::string> translation;
};

#endif