#ifndef __ACTION__
#define __ACTION__

#include <memory>
#include <unordered_set>
#include <functional>
#include "CharacterState.h"
#include "CharacterStateRecipes.h"

#define ATTACK_PRIORITY 1

class WorldObject;

// Defines the data of a game action, such as a standing attack or a dash
struct Action
{
  // Whether this action is friends with this state
  bool IsFriend(std::shared_ptr<CharacterState> state) const { return GetFriendStates().count(state->name) > 0; }

  // Whether this action can be triggered right now for this object
  virtual bool IsValid(WorldObject &) const { return true; }

  // Executed when this action is triggered
  virtual void Trigger(WorldObject &target, std::shared_ptr<CharacterState> actionState) = 0;

  // The priority this action has over the current state
  // It will be discarded and won't trigger if the state's priority is higher
  virtual int GetPriority() const = 0;

  // Returns the next state after this action triggers
  // It requires a shared pointer for this action
  virtual std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) = 0;

  // Friends don't get interrupted when this action is triggered
  // These states also don't reject this action if their priorities are higher
  virtual std::unordered_set<std::string> GetFriendStates() const { return {}; }

  // Executed when this action' state stops (either completes or is interrupted)
  // Ignored if this action yields no state
  virtual void StopHook(WorldObject &, std::shared_ptr<CharacterState>) {}

  virtual ~Action() {}

  // Simply compares if both actions have the same type
  bool operator==(const Action &other);
  bool operator!=(const Action &other);

  // How many time this action has been executed in succession prior to this execution
  int sequenceIndex{0};

  // Whether to set any states created with NextState with action input as already released
  bool actionInputAlreadyReleased{false};
};

// A specialization which simply plays an animation on trigger
struct AnimationAction : public Action
{
  // Takes over responsibility for this
  void Trigger(WorldObject &target, std::shared_ptr<CharacterState> actionState) override;

  // Requires a name for an animation
  virtual std::string GetAnimationName() const = 0;

  virtual ~AnimationAction() {}
};

// Specialization to simplify attack action constructors
struct AttackAction : public AnimationAction
{
  // Fixed priority
  int GetPriority() const override;

  // Add some functionality
  void Trigger(WorldObject &target, std::shared_ptr<CharacterState> actionState) override;

  // Fixed next state
  std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override;

  virtual ~AttackAction() {}
};

// Specialization for special
struct SpecialAction : public AttackAction
{
  // Fixed next state
  std::shared_ptr<CharacterState> NextState(std::shared_ptr<Action> sharedAction) override;

  virtual ~SpecialAction() {}
};

#endif