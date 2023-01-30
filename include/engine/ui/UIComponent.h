#ifndef __UI_COMPONENT__
#define __UI_COMPONENT__

#include "Component.h"
#include "Collision.h"
#include "TriggerCollisionData.h"

class UIComponent : public Component
{
  friend GameObject;
  friend UIObject;
  friend GameScene;

public:
  UIComponent(GameObject &associatedObject);

  virtual ~UIComponent();

  // Returns this uiComponent's shared pointer
  std::shared_ptr<UIComponent> GetShared() const;

  // The associated ui object
  UIObject &uiObject;

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

#include "UIObject.h"

#endif