#ifndef __CHARACTER__
#define __CHARACTER__

#include "GameObject.h"
#include "Component.h"
#include "CharacterState.h"
#include "CharacterStateRecipes.h"
#include "Event.h"
#include <list>
#include <unordered_set>

// An arena character that have multiple states, such as moving and jumping
class Character : public Component
{
public:
  Character(GameObject &associatedObject);
  virtual ~Character() {}

  // Triggered when a state exits normally (times out)
  EventI<std::shared_ptr<CharacterState>> OnCompleteState;

  // Triggered when an action interrupts the current state
  EventI<std::shared_ptr<CharacterState>> OnInterruptState;

  // When control is lost or recovered
  EventI<bool> OnControlChange;

  // When states becomes empty
  Event OnEnterIdle;

  // Get the current states
  const std::list<std::shared_ptr<CharacterState>> GetStates() const;

  // Add a new state to the list, and remove any states not in the keepList
  void SetState(std::shared_ptr<CharacterState> newState, std::unordered_set<std::string> keepStates = {});

  // Removes any states not in the list
  void RemoveStatesNotIn(std::unordered_set<std::string> keepStates, bool interruption = false);

  // Removes a state from it's unique id
  void RemoveState(unsigned id, bool interruption = false);

  // If the action's priority is high enough, trigger it for this character and set it's state accordingly
  // The canDelay param specifies if this action can wait until the current state is over, if it's not able to interrupt it immediately
  void Perform(std::shared_ptr<Action> action, bool canDelay = false);

  // Whether character has control
  bool HasControl() const { return hasControl; }

  // Change character control
  void SetControl(bool value);

  void Update(float) override;

private:
  void AddState(std::shared_ptr<CharacterState> newState);

  // Puts an action in the queue
  void QueueAction(std::shared_ptr<Action> action);

  // Checks if character is able to perform this action this frame
  bool CanPerform(std::shared_ptr<Action> action);

  // Sets the action's sequence index based on current state
  void SetSequenceIndex(std::shared_ptr<Action> action);

  // Action waiting to be performed as soon as there isn't an impeding state anymore
  std::shared_ptr<Action> queuedAction;

  // How much time the queued action still has to trigger before being discarded
  float queuedActionTTL{0};

  // Whether character currently has control over itself
  bool hasControl{true};

  // Current states of the character
  std::list<std::shared_ptr<CharacterState>> states;

  // Max seconds an action can stay in the queue before being discarded
  static const float maxActionDelay;

  auto RemoveState(decltype(states)::iterator stateIterator, bool interruption, bool ignoreIdleEvent = false) -> decltype(states)::iterator;
};

#endif