#ifndef __ACTIONS__
#define __ACTIONS__

#include "Action.h"
#include "CharacterStateRecipes.h"

// Action priorities
#define MOVEMENT_PRIORITY 1
#define JUMP_PRIORITY 1
#define LAND_PRIORITY 2

// Shorthand for attack actions
#define ATTACK(actionName, animationName)                               \
  struct actionName : public AttackAction                               \
  {                                                                     \
    std::string GetAnimation() const override { return animationName; } \
  }

namespace Actions
{
  struct Move : public Action
  {
    void Trigger(GameObject &target, std::shared_ptr<CharacterState> actionState) override;
    void StopHook(GameObject &target) override;
    std::unordered_set<std::string> GetFriendStates() const override { return {LANDING_STATE, JUMPING_STATE}; }

    int GetPriority() const override { return MOVEMENT_PRIORITY; }

    std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override
    {
      return direction == 0 ? nullptr : CharacterStateRecipes::Moving(sharedAction);
    }

    Move(float direction) : direction(direction) {}
    float direction;
  };

  struct Jump : public AnimationAction
  {
    std::string GetAnimation() const override { return "jump"; }
    int GetPriority() const override { return JUMP_PRIORITY; }
    std::unordered_set<std::string> GetFriendStates() const override { return {MOVING_STATE}; }

    std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override
    {
      return CharacterStateRecipes::Jumping(sharedAction);
    }
  };

  struct Land : public AnimationAction
  {
    std::string GetAnimation() const override { return "land"; }
    int GetPriority() const override { return LAND_PRIORITY; }
    std::unordered_set<std::string> GetFriendStates() const override { return {MOVING_STATE}; }

    std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override
    {
      return CharacterStateRecipes::Landing(sharedAction);
    }
  };

  ATTACK(Punch, "punch");

}

#endif