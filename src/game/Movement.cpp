#include "Movement.h"
#include "ObjectRecipes.h"
#include "Character.h"
#include "Rigidbody.h"

using namespace std;

// Interval between each valid jump input, in seconds
const float jumpCooldown{0.1f};

// How far from the ground character can jump
const float jumpRange{0.2f};

Movement::Movement(GameObject &associatedObject, float acceleration, float defaultSpeed, float feetDistance)
    : Component(associatedObject),
      acceleration(acceleration),
      defaultSpeed(defaultSpeed),
      feetDistance(feetDistance),
      jumpSpeed(28),
      doubleJumpSpeed(7),
      airborneControl(0.5f),
      fastFallAcceleration(45),
      jumpGravityModifier(15),
      rigidbody(*gameObject.RequireComponent<Rigidbody>()),
      character(*gameObject.RequireComponent<Character>())
{
  SetGravityModifierDecayTime(0.2f);
}

void Movement::SetDirection(float direction) { targetSpeed = direction * defaultSpeed; }

float Movement::GetDirection() { return targetSpeed / defaultSpeed; }

void Movement::Start()
{
  // Remember gravity scale
  originalGravityScale = rigidbody.gravityScale;
}

void Movement::OnLandInternal()
{
  // Reset gravity modifier when reaching ground
  SetCurrentGravityModifier(1);

  // Reset double jump
  doubleJumpAvailable = true;
}

void Movement::PhysicsUpdate(float deltaTime)
{
  // Discount gravity modifier
  if (currentGravityModifier > 1)
  {
    if (character.HasControl() == false)
      SetCurrentGravityModifier(1);

    else
      SetCurrentGravityModifier(max(1.0f, currentGravityModifier - gravityModifierDecay * deltaTime));
  }

  // Fast fall
  if (fastFallActive && character.HasControl())
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
  if (targetSpeed == rigidbody.velocity.x || character.HasControl() == false)
    return;

  // Use deceleration modifier
  float frameAcceleration = targetSpeed == 0 ? acceleration * decelerationModifier : acceleration;

  // Use airborne modifier
  frameAcceleration = isGrounded ? frameAcceleration : frameAcceleration * airborneControl;

  // Get the frame's acceleration vector
  Vector2 accelerationVector = Vector2::Right(targetSpeed - rigidbody.velocity.x).CapMagnitude(frameAcceleration * deltaTime);

  // Accelerate
  rigidbody.velocity += accelerationVector;

  // Velocity to use as direction reference
  float referenceSpeed = isGrounded ? rigidbody.velocity.x : targetSpeed;

  // Set object direction according to velocity
  if (referenceSpeed != 0)
    gameObject.localScale = Vector2(GetSign(referenceSpeed), 1);
}

void Movement::SetCurrentGravityModifier(float value)
{
  currentGravityModifier = value;
  rigidbody.gravityScale = originalGravityScale * currentGravityModifier;
}

void Movement::Jump()
{
  if (lastJumpTime < jumpCooldown)
    return;

  // Reset counter
  lastJumpTime = 0;

  // If not grounded
  if (IsGrounded() == false)
  {
    // Stop if can't double jump
    if (doubleJumpAvailable == false)
      return;

    // Apply double jump speed
    rigidbody.velocity.y = -doubleJumpSpeed;

    // Reset gravity modifier
    SetCurrentGravityModifier(1);

    doubleJumpAvailable = false;

    return;
  }

  // Apply speed
  rigidbody.velocity.y = -jumpSpeed;

  // Apply gravity modifier
  SetCurrentGravityModifier(jumpGravityModifier);
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

  isGrounded = rigidbody.ColliderCast(M_PI / 2, feetDistance + jumpRange, 0.9f);

  if (wasGrounded == false && isGrounded)
  {
    OnLandInternal();
    OnLand.Invoke();
  }
}

bool Movement::IsGrounded() const { return isGrounded; }

bool Movement::CanJump() const { return IsGrounded() || doubleJumpAvailable; }

void Movement::WasteDoubleJump()
{
  if (IsGrounded())
    return;

  doubleJumpAvailable = false;
}
