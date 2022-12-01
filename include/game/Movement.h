#ifndef __MOVEMENT__
#define __MOVEMENT__

#include "GameObject.h"
#include "Component.h"

class Rigidbody;

class Movement : public Component
{
public:
  Movement(GameObject &associatedObject, float acceleration, float defaultSpeed);
  virtual ~Movement() {}

  void Update(float deltaTime) override;

  // Sets the direction of movement (positive goes right, negative goes left)
  void SetDirection(float direction);
  float GetDirection();

  // Movement acceleration, in range [0, 1]. It's the proportion of targetSpeed injected each second.
  void SetAcceleration(float acceleration);
  float GetAcceleration();

  // The default max speed achieved by moving, units/s, when direction is normalized
  float defaultSpeed;

private:
  // A horizontal vector. Each frame, current velocity will be accelerated towards this
  Vector2 targetVelocity{Vector2::Zero()};

  // How much speed is retained each second
  float speedInertia;

  // Reference to rigidbody
  Rigidbody& rigidbody;
};

#endif