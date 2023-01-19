#include "CharacterController.h"
#include "LandEffector.h"
#include "Parry.h"
#include "CharacterStateRecipes.h"
#include "Action.h"
#include "Actions.h"
#include "Animation.h"
#include "PlayerInput.h"
#include <iostream>

// Min speed that trigger landing animation on land
const static float minLandAnimationSpeed{1};

using namespace std;

const float CharacterController::totalDashCooldown{1};

CharacterController::CharacterController(GameObject &associatedObject)
    : Component(associatedObject),
      stateManager(*gameObject.RequireComponent<CharacterStateManager>()),
      movement(*gameObject.RequireComponent<Movement>()),
      rigidbody(*gameObject.RequireComponent<Rigidbody>()),
      animator(*gameObject.RequireComponent<Animator>()) {}

void CharacterController::Update(float deltaTime)
{
  HandleMovementAnimation();

  // Update dash cooldown
  if (dashCooldown > 0)
    dashCooldown -= deltaTime;
}

void CharacterController::HandleMovementAnimation()
{
  // Moving animations
  auto states = stateManager.GetStates();

  // Whether just moving
  bool justMoving = states.size() == 1 && states.front()->name == "moving";

  if (movement.IsGrounded())
  {
    // When grounded, run animation if moving is the only state
    if (justMoving)
    {
      // See if moving in the same direction as input
      if (rigidbody.velocity.x == 0 || GetSign(movement.GetDirection()) == GetSign(rigidbody.velocity.x))
        animator.Play("run");

      // If not, use brake animation
      else
        animator.Play("brake");
    }

    // Otherwise, stop run animation if it's playing
    else if (animator.GetCurrentAnimation()->Name() == "run" || animator.GetCurrentAnimation()->Name() == "brake")
      // else if (animator.GetCurrentAnimation() == "run")
      animator.Play("idle");
  }

  else
    // When not grounded, rise & fall animations
    if (justMoving || states.size() == 0)
    {
      animator.Play(rigidbody.velocity.y < -0.01f ? "rise" : "fall");
    }
}

void CharacterController::Start()
{
  // For now, there must be player input. In the future there may be an AIInput instead
  auto inputs = gameObject.GetComponents<PlayerInput>();

  Assert(inputs.empty() == false, "Character object had no input methods");

  for (auto input : inputs)
  {
    auto weakInput = weak_ptr(input);

    // Subscribe to movement
    input->OnMoveDirection.AddListener("character-controller", [this](float direction)
                                       { DispatchNonDelayable<Actions::Move>(direction); });

    // Make sure to dispatch another movement if movement key is being held when character becomes idle
    stateManager.OnEnterIdle.AddListener("check-if-moving", [this, weakInput]()
                                         { if (auto input = weakInput.lock(); input && input->GetCurrentMoveDirection() != 0)
                                    DispatchNonDelayable<Actions::Move>(input->GetCurrentMoveDirection()); });

    // Subscribe to jumps
    // Make it a friend of moving
    input->OnJump.AddListener("character-controller", [this]()
                              { if (movement.CanJump() ) Dispatch<Actions::Jump>(); });

    // Fast falling isn't an action
    input->OnFastFall.AddListener("character-controller", [this]()
                                  { movement.FallFast(); });
    input->OnFastFallStop.AddListener("character-controller", [this]()
                                      { movement.StopFallFast(); });

    // Land behavior
    movement.OnLand.AddListener("character-controller", [this]()
                                { OnLand(); });

    //  Dash
    input->OnDash.AddListener("character-controller", [this](Vector2 direction)
                              { DispatchDash(direction); });

    // Attacks
    input->OnAttackNeutral.AddListener("character-controller", [this]()
                                       { Dispatch<Actions::Neutral>(); });
    input->OnAttackHorizontal.AddListener("character-controller", [this]()
                                          { Dispatch<Actions::Horizontal>(); });
    input->OnAttackUp.AddListener("character-controller", [this]()
                                  { Dispatch<Actions::Up>(); });

    // Air attacks
    input->OnAirHorizontal.AddListener("character-controller", [this]()
                                       { Dispatch<Actions::AirHorizontal>(); });
    input->OnAirUp.AddListener("character-controller", [this]()
                               { Dispatch<Actions::AirUp>(); });
    input->OnAirDown.AddListener("character-controller", [this]()
                                 { Dispatch<Actions::AirDown>(); });

    // Specials
    input->OnSpecialNeutral.AddListener("character-controller", [this]()
                                        { Dispatch<Actions::SpecialNeutral>(); });
    input->OnSpecialHorizontal.AddListener("character-controller", [this]()
                                           { Dispatch<Actions::SpecialHorizontal>(); });

    auto warnStates = [this](string targetStateName)
    {
      // For each state
      for (auto state : stateManager.GetStates())
      {
        if (state->name == targetStateName)
          state->ReleaseActionInput();
      }
    };

    // Raise release events on states
    input->OnReleaseAttack.AddListener("character-controller", [warnStates]()
                                       { warnStates(ATTACKING_STATE); });
    input->OnReleaseSpecial.AddListener("character-controller", [warnStates]()
                                        { warnStates(SPECIAL_ATTACKING_STATE); });
  }
}

void CharacterController::OnLand()
{
  // Restore air dash
  airDashAvailable = true;

  // If no control on reach ground, crash instead of landing
  if (stateManager.HasControl() == false)
  {
    DispatchNonDelayable<Actions::Crash>();
    return;
  }

  // Detect effector interception
  if (auto effectors = gameObject.GetComponents<LandEffector>(); effectors.size() > 0)
  {
    // Check if one of them will override land
    for (auto effector : effectors)
      if (auto effectorAction = effector->GetLandAction(); effectorAction != nullptr)
      {
        stateManager.Perform(effectorAction);
        return;
      }
  }

  // Perform land action if velocity is big enough
  static const float sqrMinSpeed = minLandAnimationSpeed * minLandAnimationSpeed;

  if (rigidbody.velocity.SqrMagnitude() >= sqrMinSpeed)
    DispatchNonDelayable<Actions::Land>();

  else if (stateManager.GetStates().size() == 0)
    animator.Play("idle");
}

void CharacterController::DispatchDash(Vector2 direction)
{
  // Check cooldown
  if (dashCooldown > 0)
    return;

  // Check if airborne
  if (movement.IsGrounded() == false)
  {
    if (airDashAvailable == false)
      return;

    airDashAvailable = false;

    // Also remove the double jump
    movement.WasteDoubleJump();
  }

  // If not airborne, forbid vertical dashes
  // UNLESS in jump animation!
  else if (animator.GetCurrentAnimation()->Name() != "jump")
    direction.y = 0;

  // If no direction, use object's facing direction
  if (!direction)
    direction = Vector2(gameObject.localScale.x, 0);

  dashCooldown = totalDashCooldown;

  Dispatch<Actions::Dash>(direction);
}

void CharacterController::TakeHit(Damage damage)
{
  // Check if can parry
  auto parry = gameObject.GetComponent<Parry>();

  // If can parry, perform riposte
  if (parry != nullptr && parry->CanParry(damage))
    Dispatch<Actions::Riposte>(parry, damage);

  // Otherwise, take damage
  else
    DispatchNonDelayable<Actions::TakeDamage>(damage);
}
