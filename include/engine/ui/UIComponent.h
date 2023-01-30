#ifndef __UI_COMPONENT__
#define __UI_COMPONENT__

#include "Component.h"

class UIObject;

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
};

#include "UIObject.h"

#endif