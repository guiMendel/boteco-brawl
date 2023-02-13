#include "CharacterController.h"
#include "Arena.h"
#include "LandEffector.h"
#include "Parry.h"
#include "CharacterStateRecipes.h"
#include "Action.h"
#include "Actions.h"
#include "Animation.h"
#include "PlayerInput.h"
#include <iostream>

// Min speed that triggers landing animation on land
const static float minLandAnimationSpeed{2};

// Min speed at which character crashes on ground instead of landing (only when stunned)
const static float minCrashSpeed{7};

using namespace std;

const float CharacterController::totalDashCooldown{1};

CharacterController::CharacterController(GameObject &associatedObject, shared_ptr<Player> player)
    : WorldComponent(associatedObject),
      weakPlayer(player),
      invulnerability(*worldObject.RequireComponent<Invulnerability>()),
      stateManager(*worldObject.RequireComponent<CharacterStateManager>()),
      movement(*worldObject.RequireComponent<Movement>()),
      rigidbody(*worldObject.RequireComponent<Rigidbody>()),
      animator(*worldObject.RequireComponent<Animator>()) {}

void CharacterController::PhysicsUpdate(float)
{
  lastVelocity = rigidbody.velocity;
}

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
  bool justMoving = states.size() == 1 && stateManager.HasState(MOVING_STATE);

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
      animator.Play("idle");

    // Also brake when in movement but in idle state
    if (animator.GetCurrentAnimation()->Name() == animator.defaultAnimation && abs(rigidbody.velocity.x) > 0.5f)
      animator.Play("brake");
  }

  // When not grounded
  else
  {
    // Rise & fall animations
    if (justMoving || states.size() == 0)
      animator.Play(rigidbody.velocity.y < -0.01f ? "rise" : "fall");

    // Stun spin
    if (stateManager.HasState(STUNNED_STATE) &&
        animator.GetCurrentAnimation()->Name() == animator.defaultAnimation)
      animator.Play("spin");
  }
}

void CharacterController::Start()
{
  // Enable control on battle start
  GetScene()->RequireFindComponent<Arena>()->OnBattleStart.AddListener("enable-character-control-" + to_string(id), [this]()
                                                                       { controlDisabled = false; });

  // For now, there must be player input. In the future there may be an AIInput instead
  auto inputs = worldObject.GetComponents<PlayerInput>();

  Assert(inputs.empty() == false, "Character object had no input methods");

  for (auto input : inputs)
  {
    auto weakInput = weak_ptr(input);

    // Subscribe to movement
    input->OnMoveDirection.AddListener("character-controller", [this](float direction)
                                       { Dispatch<Actions::Move>(false, true, direction); });

    // Make sure to dispatch another movement if movement key is being held when character becomes idle
    stateManager.OnEnterIdle.AddListener("check-if-moving", [this, weakInput]()
                                         { if (auto input = weakInput.lock(); input && input->GetCurrentMoveDirection() != 0)
                                    Dispatch<Actions::Move>(false, true, input->GetCurrentMoveDirection()); });

    // Subscribe to jumps
    // Make it a friend of moving
    input->OnJump.AddListener("character-controller", [this]()
                              { if (movement.CanJump() ) Dispatch<Actions::Jump>(true, true); });

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
                                       { Dispatch<Actions::Neutral>(true, true); });
    input->OnAttackHorizontal.AddListener("character-controller", [this]()
                                          { Dispatch<Actions::Horizontal>(true, true); });
    input->OnAttackUp.AddListener("character-controller", [this]()
                                  { Dispatch<Actions::Up>(true, true); });

    // Air attacks
    input->OnAirHorizontal.AddListener("character-controller", [this]()
                                       { Dispatch<Actions::AirHorizontal>(true, true); });
    input->OnAirUp.AddListener("character-controller", [this]()
                               { Dispatch<Actions::AirUp>(true, true); });
    input->OnAirDown.AddListener("character-controller", [this]()
                                 { Dispatch<Actions::AirDown>(true, true); });

    // Specials
    input->OnSpecialNeutral.AddListener("character-controller", [this]()
                                        { Dispatch<Actions::SpecialNeutral>(true, true); });
    input->OnSpecialHorizontal.AddListener("character-controller", [this]()
                                           { Dispatch<Actions::SpecialHorizontal>(true, true); });

    // Raise release events on states
    input->OnReleaseAttack.AddListener("character-controller", [this]()
                                       { AnnounceInputRelease(ATTACKING_STATE); });
    input->OnReleaseSpecial.AddListener("character-controller", [this]()
                                        { AnnounceInputRelease(SPECIAL_ATTACKING_STATE); });
  }
}

void CharacterController::AnnounceInputRelease(std::string targetState)
{
  // For each state
  for (auto state : stateManager.GetStates())
  {
    if (state->name == targetState)
      state->ReleaseActionInput();
  }

  auto queuedAction = stateManager.GetQueuedAction();

  if (queuedAction != nullptr)
  {
    auto actionState = queuedAction->NextState(queuedAction);

    if (actionState->name == targetState)
      queuedAction->actionInputAlreadyReleased = true;
  }
}

void CharacterController::OnLand()
{
  auto sqrSpeed = lastVelocity.SqrMagnitude();

  // Check if will actually just bounce right off
  if (stateManager.IsBouncing() && lastVelocity.y >= CharacterStateManager::minBounceSpeed)
    return;

  // Restore air dash
  airDashAvailable = true;

  // If stunned on reach ground, crash instead of landing
  if (stateManager.HasState(STUNNED_STATE) && sqrSpeed >= minCrashSpeed * minCrashSpeed)
  {
    Dispatch<Actions::Crash>(false, false);
    return;
  }

  // Detect effector interception
  if (auto effectors = worldObject.GetComponents<LandEffector>(); effectors.size() > 0)
  {
    // Check if one of them will override land
    for (auto effector : effectors)
      if (auto effectorAction = effector->GetLandAction(); effectorAction != nullptr)
      {
        stateManager.Perform(effectorAction);
        return;
      }
  }

  // If had no control before, restore it now
  stateManager.RemoveState(STUNNED_STATE);

  // Perform land action if velocity is big enough
  if (sqrSpeed >= minLandAnimationSpeed * minLandAnimationSpeed)
    Dispatch<Actions::Land>(false, false);

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
    direction = Vector2(worldObject.localScale.x, 0);

  dashCooldown = totalDashCooldown;

  Dispatch<Actions::Dash>(true, true, direction);
}

void CharacterController::TakeHit(Damage damage, bool parryable)
{
  // Check if can parry
  if (parryable)
  {
    auto parry = worldObject.GetComponent<Parry>();

    // If can parry, perform riposte
    if (parry != nullptr && parry->CanParry(damage))
    {
      Dispatch<Actions::Riposte>(true, false, parry, damage);
      return;
    }
  }

  // Ignore if invulnerable
  if (invulnerability.IsInvulnerable())
    return;

  // Otherwise, take damage
  Dispatch<Actions::TakeDamage>(false, false, damage);
}

shared_ptr<Player> CharacterController::GetPlayer() const
{
  LOCK(weakPlayer, player);
  return player;
}
