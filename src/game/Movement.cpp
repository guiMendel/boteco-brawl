#include "Movement.h"
#include "Rigidbody.h"

Movement::Movement(GameObject &associatedObject, float acceleration, float defaultSpeed)
    : Component(associatedObject), acceleration(acceleration), defaultSpeed(defaultSpeed), rigidbody(*gameObject.RequireComponent<Rigidbody>()) {}

void Movement::SetDirection(float direction) { targetSpeed = direction * defaultSpeed; }

float Movement::GetDirection() { return targetSpeed / defaultSpeed; }

void Movement::Update(float deltaTime)
{
  if (targetSpeed == rigidbody.velocity.x)
    return;

  // Use deceleration modifier
  float frameAcceleration = targetSpeed == 0 ? acceleration * decelerationModifier : acceleration;

  // Get the frame's acceleration vector
  Vector2 accelerationVector = Vector2::Right(targetSpeed - rigidbody.velocity.x).CapMagnitude(frameAcceleration * deltaTime);

  // Accelerate
  rigidbody.velocity += accelerationVector;
}

void Movement::Jump()
{
}

void Movement::FallFaster() {}
void Movement::FallNormally() {}