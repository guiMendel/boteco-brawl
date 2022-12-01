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

  // Iniates a jump
  void Jump();

  // Starts falling faster
  void FallFaster();

  // Stops falling faster
  void FallNormally();

  // Movement acceleration, in units/s/s
  float acceleration;

  // The default max speed achieved by moving, units/s, when direction is normalized
  float defaultSpeed;

  // Acceleration modifier applied when targetSpeed is 0 
  float decelerationModifier{0.5f};

private:
  // Each frame, current velocity will be accelerated towards this
  float targetSpeed{0};

  // Reference to rigidbody
  Rigidbody& rigidbody;
};

#endif