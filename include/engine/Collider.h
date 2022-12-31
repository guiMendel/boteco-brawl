#ifndef __COLLIDER__
#define __COLLIDER__

#include "GameObject.h"
#include "Component.h"
#include "Vector2.h"
#include "Rectangle.h"
#include "Rigidbody.h"
#include "ColliderDensity.h"

class SpriteRenderer;
class Animator;

class Collider : public Component
{
public:
  // Explicitly initialize box
  Collider(GameObject &associatedObject, Rectangle box, bool isTrigger = false, ColliderDensity density = ColliderDensity::Default);

  // Use sprite's box
  Collider(GameObject &associatedObject, std::shared_ptr<SpriteRenderer> sprite, bool isTrigger = false, ColliderDensity density = ColliderDensity::Default, Vector2 scale = Vector2::One());

  // Use sprite animator's initial animation sprite size
  Collider(GameObject &associatedObject, std::shared_ptr<Animator> animator, bool isTrigger = false, ColliderDensity density = ColliderDensity::Default, Vector2 scale = Vector2::One());

  // Use other collider's box
  Collider(GameObject &associatedObject, std::shared_ptr<Collider> other, bool isTrigger = false, ColliderDensity density = ColliderDensity::Default, Vector2 scale = Vector2::One());

  virtual ~Collider() {}

  void RegisterToState() override;
  void Render() override;

  // Get the box, with it's x & y coordinates corresponding to it's actual position in game
  Rectangle GetBox() const;
  // Set the box, assuming x & y coordinates correspond to an offset from the gameObject's position
  void SetBox(const Rectangle &box);

  float GetMaxVertexDistance() const { return maxVertexDistance; }

  RenderLayer GetRenderLayer() override { return RenderLayer::Debug; }

  float GetArea() const;
  float GetDensity() const;
  float GetMass() const { return GetArea() * GetDensity(); }

  // Tries to lock it's rigidbody component, and throws if it's expired
  std::shared_ptr<Rigidbody> RequireRigidbody() const;

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

#include "SpriteRenderer.h"
#include "Animator.h"

#endif