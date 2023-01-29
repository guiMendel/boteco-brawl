#ifndef __WORLD_COMPONENT__
#define __WORLD_COMPONENT__

#include "Component.h"
#include "Collision.h"
#include "TriggerCollisionData.h"

class WorldComponent : public Component
{
  friend GameObject;
  friend WorldObject;
  friend GameScene;

public:
  WorldComponent(GameObject &associatedObject);

  virtual ~WorldComponent();

  // Returns this worldComponent's shared pointer
  std::shared_ptr<WorldComponent> GetShared() const;

  // The associated world object
  WorldObject &worldObject;

protected:
  // Allows for reacting to collision
  virtual void OnCollision(Collision::Data) {}
  virtual void OnCollisionEnter(Collision::Data) {}
  virtual void OnCollisionExit(Collision::Data) {}

  // Allows for reacting to trigger collision
  virtual void OnTriggerCollision(TriggerCollisionData) {}
  virtual void OnTriggerCollisionEnter(TriggerCollisionData) {}
  virtual void OnTriggerCollisionExit(TriggerCollisionData) {}
};

#include "WorldObject.h"

#endif