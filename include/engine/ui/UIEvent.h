#ifndef __UI_EVENT__
#define __UI_EVENT__

#include "Helper.h"
#include "Vector2.h"

class UIEvent
{
public:
  // Defines the possible types of ui events
  enum Type
  {
    OnMouseEnter,
    OnMouseLeave,
    OnMouseOver,
    OnMouseClick
  };

  // This event's type
  virtual Type GetType() = 0;

  // Whether it is propagating
  bool propagating{true};
};

class UIMouseEvent : public UIEvent
{
public:
  UIMouseEvent(Type type, Vector2 screenPosition);

  Type GetType() override { return type; }

  Vector2 screenPosition;

private:
  Type type;
};

#endif