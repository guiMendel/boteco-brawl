#ifndef __MINION__
#define __MINION__

#include "GameObject.h"
#include "Component.h"
#include "Vector2.h"

#include <memory>

class Minion : public Component
{
public:
  // Speed at which to orbit the host alien, in radians
  static const float angularSpeed;

  // Speed at which the radius changes
  static const float radiusFloatSpeed;

  // Limits to radius floating
  static const float radiusLimits[2];

  // Limits to scale
  static const float scaleLimits[2];

  // Projectile speed
  static const float projectileSpeed;

  // Projectile time to live, in seconds
  static const float projectileTimeToLive;

  // Damage of projectile
  static const float projectileDamage;

  // How many health points it has
  static const float healthPoints;

  Minion(GameObject &associatedObject, std::weak_ptr<GameObject> hostPointer, float startingArc = 0);
  
  virtual ~Minion() {}

  void OnBeforeDestroy() override;
  void Update(float deltaTime) override;
  RenderLayer GetRenderLayer() override { return RenderLayer::None; }

  void Shoot(Vector2 target);

private:
  // Alien object around which to orbit
  std::weak_ptr<GameObject> hostPointer;

  // Where in the orbit's circumference the minion currently is
  float arc;

  // Distance from the orbit center
  float orbitRadius;

  // Whether to float away or towards the center (1 or -1)
  float floatDirection{1};
};

#endif