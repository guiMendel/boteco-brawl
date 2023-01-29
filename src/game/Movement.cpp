#include "Movement.h"
#include "FallDeath.h"
#include "ObjectRecipes.h"
#include "CharacterStateManager.h"
#include "Rigidbody.h"

using namespace std;

// Interval between each valid jump input, in seconds
const float jumpCooldown{0.1f};

// How far from the ground character can jump
const float groundedRange{0.2f};

const float Movement::totalCoyoteTime{0.2f};

Movement::Movement(GameObject &associatedObject, float acceleration, float defaultSpeed, float feetDistance)
    : WorldComponent(associatedObject),
      acceleration(acceleration),
      defaultSpeed(defaultSpeed),
      feetDistance(feetDistance),
      jumpSpeed(28),
      doubleJumpSpeed(7),
      airborneControl(0.5f),
      fastFallAcceleration(45),
      jumpGravityModifier(15),
      rigidbody(*worldObject.RequireComponent<Rigidbody>()),
      stateManager(*worldObject.RequireComponent<CharacterStateManager>())
{
  SetGravityModifierDecayTime(0.2f);
}

void Movement::SetDirection(float direction) { targetSpeed = direction * defaultSpeed; }

float Movement::GetDirection() { return targetSpeed / defaultSpeed; }

void Movement::Start()
{
  // Remember gravity scale
  originalGravityScale = rigidbody.gravityScale;

  // On death, reset fast fall & direction
  worldObject.RequireComponent<FallDeath>()->OnFall.AddListener("reset-movement", [this]()
                                                                   { SetDirection(0); StopFallFast(); });
}

void Movement::OnLandInternal()
{
  // Reset gravity modifier when reaching ground
  SetCurrentGravityModifier(1);

  // Reset double jump
  doubleJumpAvailable = true;

  // Reset coyote time
  remainingCoyoteTime = totalCoyoteTime;
}

void Movement::PhysicsUpdate(float deltaTime)
{
  // Update grounded state
  GroundCheck();

  // Discount gravity modifier
  if (currentGravityModifier > 1)
  {
    if (stateManager.HasControl() == false)
      SetCurrentGravityModifier(1);

    else
      SetCurrentGravityModifier(max(1.0f, currentGravityModifier - gravityModifierDecay * deltaTime));
  }

  // Fast fall
  if (fastFallActive && stateManager.HasControl())
    rigidbody.velocity.y += fastFallAcceleration * deltaTime;
}

void Movement::Update(float deltaTime)
{
  // Update coyote timer
  if (IsGrounded() == false && remainingCoyoteTime > 0)
    remainingCoyoteTime -= deltaTime;

  // Count jump time
  lastJumpTime += deltaTime;

  // Horizontal movement
  Run(deltaTime);
}

void Movement::Run(float deltaTime)
{
  if (targetSpeed == rigidbody.velocity.x || stateManager.HasState(STUNNED_STATE))
    return;

  // Use deceleration modifier
  float frameAcceleration = targetSpeed == 0 ? acceleration * decelerationModifier : acceleration;

  // Use airborne modifier
  frameAcceleration = isGrounded ? frameAcceleration : frameAcceleration * airborneControl;

  // Get the frame's acceleration vector
  Vector2 accelerationVector = Vector2::Right(targetSpeed - rigidbody.velocity.x).CapMagnitude(frameAcceleration * deltaTime);

  // Accelerate
  rigidbody.velocity += accelerationVector;

  // === SET ORIENTATION

  if (targetSpeed == 0)
    return;

  // When airborne
  if (IsGrounded() == false)
  {
    // Only sets if not attacking
    if (stateManager.HasState(AIR_ATTACKING_STATE) == false)
      worldObject.localScale = Vector2(GetSign(targetSpeed), 1);
  }

  // When grounded
  else
  {
    // Only set if moving in same direction as input
    if (GetSign(targetSpeed) == GetSign(rigidbody.velocity.x, 0))
      worldObject.localScale = Vector2(GetSign(targetSpeed), 1);
  }
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

  // If not grounded and coyote jump not available
  if (IsGrounded() == false && remainingCoyoteTime <= 0)
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

  // Spend coyote jump
  remainingCoyoteTime = 0;

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

  isGrounded = rigidbody.ColliderCast(M_PI / 2, groundedRange, 0.9f);

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

float Movement::GetTargetSpeed() const { return targetSpeed; }
