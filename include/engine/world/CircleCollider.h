#ifndef __CIRCLE_COLLIDER__
#define __CIRCLE_COLLIDER__

#include "Collider.h"
#include "Camera.h"
#include "Sprite.h"
#include "Circle.h"
#include <iostream>

class CircleCollider : public Collider
{
public:
  // Explicitly initialize circle
  CircleCollider(WorldObject &associatedObject, Circle circle, bool isTrigger = false, ColliderDensity density = ColliderDensity::Default);

  virtual ~CircleCollider() {}

  // Set the circle, assuming coordinates & rotation correspond to an offset from the worldObject's
  void SetCircle(const Circle &circle);

  // Get shape as circle
  Circle GetCircle() const;

  // Allows for debug rendering
  void Render() override;

protected:
  // Create empty circle
  std::shared_ptr<Shape> CopyShape() const override;
};

#endif