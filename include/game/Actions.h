#ifndef __ACTIONS__
#define __ACTIONS__

#include "Action.h"
#include "Vector2.h"
#include "CharacterStateRecipes.h"
#include "Damage.h"
#include "Parry.h"
#include "Movement.h"

// Action priorities
#define MOVEMENT_PRIORITY 1
#define DASH_PRIORITY 1
#define TAKE_DAMAGE_PRIORITY 2
#define JUMP_PRIORITY 1
#define LAND_PRIORITY 2

// Action timer names
#define STUN_DURATION_TIMER "stun-duration"

// Shorthand for attack actions
#define ATTACK(actionName, animationName)                       \
  struct actionName : public AttackAction                       \
  {                                                             \
    std::string GetAnimationName() const override               \
    {                                                           \
      return animationName;                                     \
    }                                                           \
    bool IsValid(GameObject &target) const override             \
    {                                                           \
      return target.RequireComponent<Movement>()->IsGrounded(); \
    }                                                           \
  }

#define AIR_ATTACK(actionName, animationName)                                                \
  struct actionName : public AttackAction                                                    \
  {                                                                                          \
    std::string GetAnimationName() const override                                            \
    {                                                                                        \
      return animationName;                                                                  \
    }                                                                                        \
    std::unordered_set<std::string> GetFriendStates() const override                         \
    {                                                                                        \
      return {MOVING_STATE};                                                                 \
    }                                                                                        \
    std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override \
    {                                                                                        \
      return CharacterStateRecipes::AirAttacking(sharedAction);                              \
    }                                                                                        \
    bool IsValid(GameObject &target) const override                                          \
    {                                                                                        \
      return target.RequireComponent<Movement>()->IsGrounded() == false;                     \
    }                                                                                        \
  }

#define SPECIAL(actionName, animationName)        \
  struct actionName : public SpecialAction        \
  {                                               \
    std::string GetAnimationName() const override \
    {                                             \
      return animationName;                       \
    }                                             \
  }

namespace Actions
{
  struct Move : public Action
  {
    void Trigger(GameObject &target, std::shared_ptr<CharacterState> actionState) override;
    void StopHook(GameObject &target, std::shared_ptr<CharacterState> actionState) override;
    std::unordered_set<std::string> GetFriendStates() const override { return {LANDING_STATE, JUMPING_STATE, AIR_ATTACKING_STATE}; }

    int GetPriority() const override { return MOVEMENT_PRIORITY; }

    std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override
    {
      return direction == 0 ? nullptr : CharacterStateRecipes::Moving(sharedAction);
    }

    Move(float direction) : direction(direction) {}
    float direction;
  };

  struct Dash : public Action
  {
    void Trigger(GameObject &target, std::shared_ptr<CharacterState> actionState) override;
    void StopHook(GameObject &target, std::shared_ptr<CharacterState> actionState) override;

    int GetPriority() const override { return DASH_PRIORITY; }

    std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override
    {
      return CharacterStateRecipes::Dashing(sharedAction);
    }

    Dash(Vector2 direction) : direction(direction) {}
    Vector2 direction;

    // Params
    static const float dashFriction;
    static const float dashSpeed;
  };

  struct TakeDamage : public Action
  {
    void Trigger(GameObject &target, std::shared_ptr<CharacterState> actionState) override;

    int GetPriority() const override { return TAKE_DAMAGE_PRIORITY; }

    std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override
    {
      return damage.stunTime > 0 ? CharacterStateRecipes::Stunned(sharedAction) : nullptr;
    }

    TakeDamage(Damage damage) : damage(damage) {}
    Damage damage;
  };

  struct Jump : public AnimationAction
  {
    std::string GetAnimationName() const override { return "jump"; }
    int GetPriority() const override { return JUMP_PRIORITY; }
    std::unordered_set<std::string> GetFriendStates() const override { return {MOVING_STATE}; }

    std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override
    {
      return CharacterStateRecipes::Jumping(sharedAction);
    }
  };

  struct Land : public AnimationAction
  {
    std::string GetAnimationName() const override { return "land"; }
    int GetPriority() const override { return LAND_PRIORITY; }
    std::unordered_set<std::string> GetFriendStates() const override { return {MOVING_STATE}; }

    std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override
    {
      return CharacterStateRecipes::Landing(sharedAction);
    }
  };

  // Like land, but falls down on the ground
  struct Crash : public AnimationAction
  {
    std::string GetAnimationName() const override { return "crash"; }
    int GetPriority() const override { return LAND_PRIORITY; }

    std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override
    {
      return CharacterStateRecipes::Recovering(sharedAction);
    }
  };

  // Ground attacks
  ATTACK(Neutral, "neutral");
  ATTACK(Horizontal, "horizontal");
  ATTACK(Up, "up");

  // Air attacks
  AIR_ATTACK(AirHorizontal, "airHorizontal");
  AIR_ATTACK(AirUp, "airUp");
  AIR_ATTACK(AirDown, "airDown");

  // Specials
  SPECIAL(SpecialNeutral, "specialNeutral");
  SPECIAL(SpecialHorizontal, "specialHorizontal");

  // === SPECIAL ACTIONS

  // Triggered by a successful parry
  struct Riposte : public Action
  {
    void Trigger(GameObject &target, std::shared_ptr<CharacterState> actionState) override;

    int GetPriority() const override { return ATTACK_PRIORITY; }

    std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override
    {
      return CharacterStateRecipes::Attacking(sharedAction);
    }

    Riposte(std::shared_ptr<Parry> parry, Damage parriedDamage) : weakParry(parry), parriedDamage(parriedDamage) {}

    std::weak_ptr<Parry> weakParry;
    Damage parriedDamage;
  };

  // Triggered by attacks that are meant to hit the ground
  struct LandingAttack : public Action
  {
    void Trigger(GameObject &target, std::shared_ptr<CharacterState> actionState) override;

    int GetPriority() const override { return LAND_PRIORITY; }

    std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override
    {
      return CharacterStateRecipes::Attacking(sharedAction);
    }

    LandingAttack(float landingSpeed) : landingSpeed(landingSpeed) {}

    float landingSpeed;
  };
}

#endif