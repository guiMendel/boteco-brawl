#ifndef __BULLET__
#define __BULLET__

#include "GameObject.h"
#include "Component.h"
#include "Vector2.h"
#include <memory>

class Projectile : public Component
{
public:
  Projectile(
      GameObject &associatedObject,
      float startingAngle,
      float speed = 300.0f,
      float timeToLive = 5.0f,
      std::weak_ptr<GameObject> target = std::weak_ptr<GameObject>(),
      float chaseSteering = 0.5f);

  virtual ~Projectile() {}

  void Update(float deltaTime) override;

private:
  void Chase();

  // Current speed
  Vector2 speed;

  // Current angle
  float angle;

  // How much tim it still has to live
  float timeToLive;

  // A target to chase (if empty, goes straight)
  std::weak_ptr<GameObject> targetWeak;

  // Chase steering power, in radians
  float chaseSteering;
};

#endif