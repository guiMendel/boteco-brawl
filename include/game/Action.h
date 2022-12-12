#ifndef __ACTION__
#define __ACTION__

#include <memory>
#include <unordered_set>
#include <functional>
#include "CharacterState.h"
#include "CharacterStateRecipes.h"

class GameObject;

// Defines the data of a game action, such as a standing attack or a dash
struct Action
{
  using state_getter = std::function<std::shared_ptr<CharacterState>(std::shared_ptr<Action>)>;
  using Callback = std::function<void(GameObject &, std::shared_ptr<CharacterState>)>;

  // Whether this action is friends with this state
  bool IsFriend(std::shared_ptr<CharacterState> state) const { return friendStates.count(state->identifier) > 0; }

  // Executed when this action is triggered
  const Callback callback;

  // Executed when this action' state stops (either completes or is interrupted)
  // Ignored if this action yields no state
  std::function<void(GameObject &)> stopCallback;

  // The priority this action has over the current state
  // It will be discarded and won't trigger if the state's priority is higher
  const int priority;

  // Returns the next state after this action triggers
  const state_getter getState;

  // States that don't get interrupted when this action is triggered
  // These states also don't reject this action if their priorities are higher
  const std::unordered_set<std::string> friendStates;

  Action(Callback callback, int priority, state_getter getState, std::unordered_set<std::string> friendStates = {});
};

// A specialization which simply plays an animation on trigger
struct AnimationAction : public Action
{
  void Trigger(std::string animation, GameObject &gameObject, std::shared_ptr<CharacterState> state);

  AnimationAction(std::string animation, int priority, state_getter getState, std::unordered_set<std::string> friendStates = {});
};

// Specialization to simplify attack action constructors
struct AttackAction : public AnimationAction
{
  AttackAction(std::string animation, std::unordered_set<std::string> friendStates = {});
};

#endif