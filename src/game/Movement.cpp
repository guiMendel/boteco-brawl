#include "Movement.h"
#include "Rigidbody.h"

Movement::Movement(GameObject &associatedObject, float acceleration, float defaultSpeed)
    : Component(associatedObject), defaultSpeed(defaultSpeed), rigidbody(*gameObject.RequireComponent<Rigidbody>())
{
  SetAcceleration(acceleration);
}

void Movement::SetDirection(float direction) { targetVelocity = Vector2::Right(direction * defaultSpeed); }

float Movement::GetDirection() { return targetVelocity.x / defaultSpeed; }

void Movement::SetAcceleration(float acceleration)
{
  Assert(acceleration >= 0 && acceleration <= 1, "Acceleration must be in range [0, 1]");
  speedInertia = 1 - acceleration;
}

float Movement::GetAcceleration() { return 1 - speedInertia; }

void Movement::Update(float deltaTime)
{
  // Make acceleration proportional do deltaTime
  float frameSpeedInertia = pow(speedInertia, deltaTime);

  rigidbody.velocity = rigidbody.velocity * frameSpeedInertia + targetVelocity * (1 - speedInertia);
}
