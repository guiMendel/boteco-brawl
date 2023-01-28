#ifndef __BOX_COLLIDER__
#define __BOX_COLLIDER__

#include "Collider.h"
#include "Camera.h"
#include "Sprite.h"
#include "Rectangle.h"
#include <iostream>

class BoxCollider : public Collider
{
public:
  // Explicitly initialize box
  BoxCollider(WorldObject &associatedObject, Rectangle box, bool isTrigger = false, ColliderDensity density = ColliderDensity::Default);

  // Use sprite's box
  BoxCollider(WorldObject &associatedObject, std::shared_ptr<Sprite> sprite, bool isTrigger = false, ColliderDensity density = ColliderDensity::Default, Vector2 scale = Vector2::One());

  // Use sprite animator's initial animation sprite size
  BoxCollider(WorldObject &associatedObject, std::shared_ptr<Animator> animator, bool isTrigger = false, ColliderDensity density = ColliderDensity::Default, Vector2 scale = Vector2::One());

  // Use other BoxCollider's box
  BoxCollider(WorldObject &associatedObject, std::shared_ptr<BoxCollider> other, bool isTrigger = false, ColliderDensity density = ColliderDensity::Default, Vector2 scale = Vector2::One());

  virtual ~BoxCollider() {}

  // Allows for debug rendering
  void Render() override;

  // Set the box, assuming coordinates & rotation correspond to an offset from the worldObject's
  void SetBox(const Rectangle &box);

  // Get shape as rectangle
  Rectangle GetBox() const;

protected:
  // Create empty rectangle
  std::shared_ptr<Shape> CopyShape() const override;
};

#endif