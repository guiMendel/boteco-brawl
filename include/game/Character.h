#ifndef __CHARACTER__
#define __CHARACTER__

#include "GameObject.h"
#include "Component.h"
#include "CharacterState.h"
#include "CharacterStateRecipes.h"
#include "Event.h"
#include <list>
#include <unordered_set>

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
  void RemoveStatesNotIn(std::unordered_set<std::string> keepStates);

  // Removes a state from it's unique id
  void RemoveState(unsigned id);

  // If the action's priority is high enough, trigger it for this character and set it's state accordingly
  void Perform(std::shared_ptr<Action> action);

  // Whether character has control
  bool HasControl() const { return hasControl; }

private:
  void AddState(std::shared_ptr<CharacterState> newState);

  // Whether character currently has control over itself
  bool hasControl{true};

  // Current states of the character
  std::list<std::shared_ptr<CharacterState>> states;

  auto RemoveState(decltype(states)::iterator stateIterator, bool ignoreIdleEvent = false) -> decltype(states)::iterator;
};

#endif