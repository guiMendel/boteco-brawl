#ifndef __COLLIDER__
#define __COLLIDER__

#include "GameObject.h"
#include "Component.h"
#include "Vector2.h"
#include "Rectangle.h"
#include "Rigidbody.h"
#include "ColliderDensity.h"

class Sprite;
class SpriteAnimator;

class Collider : public Component
{
public:
  // Explicitly initialize box
  Collider(GameObject &associatedObject, Rectangle box, bool isTrigger = false, ColliderDensity density = ColliderDensity::Default);

  // Use sprite's box
  Collider(GameObject &associatedObject, std::shared_ptr<Sprite> sprite, bool isTrigger = false, ColliderDensity density = ColliderDensity::Default, Vector2 scale = Vector2::One());

  // Use sprite animator's frame size
  Collider(GameObject &associatedObject, std::shared_ptr<SpriteAnimator> animator, bool isTrigger = false, ColliderDensity density = ColliderDensity::Default, Vector2 scale = Vector2::One());

  virtual ~Collider() {}

  void Start() override;
  void Render() override;

  // Get the box, with it's x & y coordinates corresponding to it's actual position in game
  Rectangle GetBox() const;
  // Set the box, assuming x & y coordinates correspond to an offset from the gameObject's position
  void SetBox(const Rectangle &box);

  float GetMaxVertexDistance() const { return maxVertexDistance; }

  RenderLayer GetRenderLayer() override { return RenderLayer::Debug; }

  float GetArea() const;
  float GetDensity() const;
  float GetMass() const { return GetArea() * GetMass(); }

  // Whether this collider actually participates in physical collisions
  const bool isTrigger;

  // Density associated to this collider
  ColliderDensity density;

  // Rigidbody associated to this collider (null if trigger)
  std::weak_ptr<Rigidbody> rigidbodyWeak;

private:
  // Collision detection area (x & y coordinates dictate the offset of the box from the object's position)
  Rectangle box;

  // Maximum distance of a vertex from the rectangle's center
  float maxVertexDistance;
};

#include "Sprite.h"
#include "SpriteAnimator.h"

#endif