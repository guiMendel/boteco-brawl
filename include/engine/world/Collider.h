#ifndef __COLLIDER__
#define __COLLIDER__

#include "WorldObject.h"
#include "Component.h"
#include "Vector2.h"
#include "Shape.h"
#include "Rigidbody.h"
#include "ColliderDensity.h"

class SpriteRenderer;
class Animator;

class Collider : public Component
{
public:
  // Explicitly initialize shape
  Collider(
      WorldObject &associatedObject,
      std::shared_ptr<Shape> shape,
      bool isTrigger = false,
      ColliderDensity density = ColliderDensity::Default);

  virtual ~Collider() {}

  RenderLayer GetRenderLayer() override { return RenderLayer::Debug; }

  // Registers this collider in the physics collider structure
  void RegisterToScene() override;

  // Allows for debug rendering
  virtual void Render() override {}

  // Get density value
  float GetDensity() const;

  // Calculate's mass from density and shape area
  float GetMass() const;

  // Tries to lock it's rigidbody component, and throws if it's expired
  std::shared_ptr<Rigidbody> RequireRigidbody() const;

  // Id of object under which this collider is registered
  int GetOwnerId() const;

  // Get the associated shape, already rotated, scaled and displaced to this worldObject's scale, rotation and position
  std::shared_ptr<Shape> DeriveShape() const;

  // Get the owner world object
  std::shared_ptr<WorldObject> GetOwner() const;

  // Whether this collider actually participates in physical collisions
  const bool isTrigger;

  // Density associated to this collider
  ColliderDensity density;

  // Rigidbody associated to this collider (null if trigger)
  std::weak_ptr<Rigidbody> rigidbodyWeak;

  // Collision detection area (coordinates & rotation are an offset from the object's)
  std::shared_ptr<Shape> shape;

protected:
  // Create a copy of this shape, for the collider's inherited type. The shared ptr must have a separate counter
  virtual std::shared_ptr<Shape> CopyShape() const = 0;

private:
  // Id of the owner worldObject
  int ownerId;
};

#include "SpriteRenderer.h"
#include "Animator.h"

#endif