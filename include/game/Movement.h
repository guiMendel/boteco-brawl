#ifndef __MOVEMENT__
#define __MOVEMENT__

#include "GameObject.h"
#include "Component.h"

class Rigidbody;
class CharacterStateManager;

class Movement : public Component
{
public:
  // Triggers when character touches the ground
  Event OnLand;

  Movement(GameObject &associatedObject, float acceleration, float defaultSpeed, float feetDistance);
  virtual ~Movement() {}

  void Start() override;
  void Update(float deltaTime) override;
  void PhysicsUpdate(float deltaTime) override;

  // Sets the direction of movement (positive goes right, negative goes left)
  void SetDirection(float direction);
  float GetDirection();

  // Iniates a jump
  void Jump();

  // Starts falling faster
  void FallFast();

  // Stops falling faster
  void StopFallFast();

  // Set gravity modifier applied on jump
  void SetJumpGravityModifier(float value);

  // Whether is grounded
  bool IsGrounded() const;

  // Whether character can jump this frame
  bool CanJump() const;

  // Wastes the double jump, making it so it can't be used until landing
  void WasteDoubleJump();

  float GetTargetSpeed() const;

  // === HORIZONTAL MOVEMENT

  // Movement acceleration, in units/s/s
  float acceleration;

  // The default max speed achieved by moving, units/s, when direction is normalized
  float defaultSpeed;

  // Acceleration modifier applied when targetSpeed is 0
  float decelerationModifier{0.5f};

  // === JUMPS

  // How far from the gameObject center the feet are
  float feetDistance;

  // Speed to be injected on jump
  float jumpSpeed;

  // Speed to be injected on double jump
  float doubleJumpSpeed;

  // How much control character has of horizontal speed when airborne
  float airborneControl;

  // Acceleration added when falling faster
  float fastFallAcceleration;

  // How long character still has to perform a coyote jump after having left ground
  float remainingCoyoteTime{totalCoyoteTime};

private:
  // Checks if character is grounded
  void GroundCheck();

  // Performs a walk cycle
  void Run(float deltaTime);

  // How long gravity modifier should take to completely vanish, in seconds
  void SetGravityModifierDecayTime(float newValue);

  // Triggered on landing
  void OnLandInternal();

  void SetCurrentGravityModifier(float value);

  // === HORIZONTAL MOVEMENT

  // Each frame, current velocity will be accelerated towards this
  float targetSpeed{0};

  // === JUMPS

  // Time elapsed since last jump
  float lastJumpTime{0};

  // What the current gravity modifier is
  float currentGravityModifier{1};

  // Gravity modifier to be applied and rapidly decayed on jump
  float jumpGravityModifier;

  // Decay of gravity modifier
  float gravityModifierDecay;

  // Take note of the original gravity scale
  Vector2 originalGravityScale;

  // Whether character is grounded this frame
  bool isGrounded{false};

  // Whether character was grounded last frame
  bool wasGrounded{false};

  // Whether fast fall is active
  bool fastFallActive{false};

  // Whether second jump is available
  bool doubleJumpAvailable{true};

  // Total time to perform coyote jump
  static const float totalCoyoteTime;

  // === OTHER

  // Reference to rigidbody
  Rigidbody &rigidbody;
  CharacterStateManager &character;
};

#endif