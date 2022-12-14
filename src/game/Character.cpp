#include "Character.h"
#include "Action.h"
#include <algorithm>

using namespace std;

Character::Character(GameObject &associatedObject)
    : Component(associatedObject) {}

const list<shared_ptr<CharacterState>> Character::GetStates() const { return states; }
void Character::SetState(shared_ptr<CharacterState> newState, unordered_set<string> keepStates)
{
  // Check if new state is in keep states
  if (keepStates.count(newState->name) > 0)
    keepStates.erase(newState->name);

  RemoveStatesNotIn(keepStates);

  states.push_back(newState);
}

void Character::RemoveStatesNotIn(std::unordered_set<std::string> keepStates)
{
  if (keepStates.size() == 0)
    states.clear();

  else
  {
    auto stateIterator = states.begin();
    while (stateIterator != states.end())
    {
      auto state = *stateIterator;

      // If it's in the list, continue
      if (keepStates.count(state->name) > 0)
        stateIterator++;

      // Otherwise, remove it
      else
      {
        // Call it's stop callback if necessary
        auto parentAction = state->parentAction;
        if (parentAction != nullptr && parentAction->stopCallback)
          parentAction->stopCallback(gameObject);

        // Announce interruption
        OnInterruptState.Invoke(state);

        stateIterator = states.erase(stateIterator);
      }
    }
  }
}

void Character::Perform(shared_ptr<Action> action)
{
  // Check if this action has lower priority than a state which IS NOT in it's friends list
  if (any_of(states.begin(), states.end(), [action](shared_ptr<CharacterState> state)
             { return state->priority > action->priority && !action->IsFriend(state); }))
    return;

  // This action's yielded state
  shared_ptr<CharacterState> newState;

  // Set new state
  if (action->getState)
  {
    newState = action->getState(action);
    SetState(newState, action->friendStates);
  }

  // If no new state, simply remove those not in the list
  else
    RemoveStatesNotIn(action->friendStates);

  // Trigger this action
  action->callback(gameObject, newState);
}

void Character::RemoveState(unsigned id)
{
  // Find this state
  auto stateIterator = find_if(states.begin(), states.end(), [id](shared_ptr<CharacterState> state)
                            { return state->id == id; });

  //  If it's not there, stop
  if (stateIterator == states.end())
    return;

  // Remove it
  states.erase(stateIterator);
}
