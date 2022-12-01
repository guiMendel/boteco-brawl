#include "Movement.h"
#include "Rigidbody.h"

using namespace std;

// Interval between each valid jump input, in seconds
const float jumpCooldown{0.2f};

// How far from the ground character can jump
const float jumpRange{0.2f};

Movement::Movement(GameObject &associatedObject, float acceleration, float defaultSpeed, float feetDistance)
    : Component(associatedObject),
      acceleration(acceleration),
      defaultSpeed(defaultSpeed),
      feetDistance(feetDistance),
      jumpSpeed(24),
      airborneControl(0.5f),
      fastFallAcceleration(45),
      jumpGravityModifier(15),
      rigidbody(*gameObject.RequireComponent<Rigidbody>())
{
  SetGravityModifierDecayTime(0.2f);
}

void Movement::SetDirection(float direction) { targetSpeed = direction * defaultSpeed; }

float Movement::GetDirection() { return targetSpeed / defaultSpeed; }

void Movement::Start()
{
  originalGravityScale = rigidbody.gravityScale;

  // Reset gravity modifier when reaching ground
  OnReachGround.AddListener("gravity-modifier-reset", [this]()
                            { currentGravityModifier = 1;
                            rigidbody.gravityScale = originalGravityScale; });
}

void Movement::PhysicsUpdate(float deltaTime)
{
  // Discount gravity modifier
  if (currentGravityModifier > 1)
  {
    currentGravityModifier = max(1.0f, currentGravityModifier - gravityModifierDecay * deltaTime);
    rigidbody.gravityScale = originalGravityScale * currentGravityModifier;
  }

  // Fast fall
  if (fastFallActive)
    rigidbody.velocity.y += fastFallAcceleration * deltaTime;
}

void Movement::Update(float deltaTime)
{
  // Update grounded state
  GroundCheck();

  // Count jump time
  lastJumpTime += deltaTime;

  // Horizontal movement
  Run(deltaTime);
}

void Movement::Run(float deltaTime)
{
  if (targetSpeed == rigidbody.velocity.x)
    return;

  // Use deceleration modifier
  float frameAcceleration = targetSpeed == 0 ? acceleration * decelerationModifier : acceleration;

  // Use airborne modifier
  frameAcceleration = isGrounded ? frameAcceleration : frameAcceleration * airborneControl;

  // Get the frame's acceleration vector
  Vector2 accelerationVector = Vector2::Right(targetSpeed - rigidbody.velocity.x).CapMagnitude(frameAcceleration * deltaTime);

  // Accelerate
  rigidbody.velocity += accelerationVector;
}

void Movement::Jump()
{
  if (isGrounded == false || lastJumpTime < jumpCooldown)
    return;

  // Reset counter
  lastJumpTime = 0;

  // Apply speed
  rigidbody.velocity.y -= jumpSpeed;

  // Apply gravity modifier
  currentGravityModifier = jumpGravityModifier;
  rigidbody.gravityScale = originalGravityScale * currentGravityModifier;
}

void Movement::FallFast()
{
  fastFallActive = true;
}

void Movement::StopFallFast()
{
  fastFallActive = false;
}

void Movement::SetGravityModifierDecayTime(float newValue)
{
  gravityModifierDecay = (jumpGravityModifier - 1) / newValue;
}

void Movement::SetJumpGravityModifier(float newValue)
{
  // Need to keep this constant
  float decayTime = (jumpGravityModifier - 1) / gravityModifierDecay;

  // Update it
  jumpGravityModifier = newValue;

  // Keep the thing constant
  SetGravityModifierDecayTime(decayTime);
}

void Movement::GroundCheck()
{
  // Update old value
  wasGrounded = isGrounded;

  isGrounded = rigidbody.Raycast(M_PI / 2, feetDistance + jumpRange);

  if (wasGrounded == false && isGrounded)
    OnReachGround.Invoke();
}
