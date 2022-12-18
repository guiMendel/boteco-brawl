#include "Character.h"
#include "Action.h"
#include <algorithm>

using namespace std;

Character::Character(GameObject &associatedObject)
    : Component(associatedObject) {}

const list<shared_ptr<CharacterState>> Character::GetStates() const { return states; }
void Character::SetState(shared_ptr<CharacterState> newState, unordered_set<string> keepStates)
{
  // Add it
  AddState(newState);

  // Make sure not to remove it right away
  keepStates.insert(newState->name);

  // Remove other states
  RemoveStatesNotIn(keepStates);
}

void Character::RemoveStatesNotIn(std::unordered_set<std::string> keepStates)
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
      if (parentAction != nullptr)
        parentAction->StopHook(gameObject);

      // Announce interruption
      OnInterruptState.Invoke(state);

      stateIterator = RemoveState(stateIterator);
    }
  }
}

void Character::Perform(shared_ptr<Action> action)
{
  // Check if this action has lower priority than a state which IS NOT in it's friends list
  if (any_of(states.begin(), states.end(), [action](shared_ptr<CharacterState> state)
             { return state->priority > action->GetPriority() && !action->IsFriend(state); }))
    return;

  // This action's yielded state
  shared_ptr<CharacterState> newState = action->NextState(action);

  // Set new state
  if (newState != nullptr)
    SetState(newState, action->GetFriendStates());

  // If no new state, simply remove those not in the list
  else
    RemoveStatesNotIn(action->GetFriendStates());

  // Trigger this action
  action->Trigger(gameObject, newState);
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
  RemoveState(stateIterator);
}

auto Character::RemoveState(decltype(states)::iterator stateIterator, bool ignoreIdleEvent) -> decltype(states)::iterator
{
  // Remove it
  auto newIterator = states.erase(stateIterator);

  // Check for raise
  if (ignoreIdleEvent == false && states.empty())
    OnEnterIdle.Invoke();

  return newIterator;
}

void Character::AddState(std::shared_ptr<CharacterState> newState)
{
  // If it's already there, remove it first
  auto stateIterator = find_if(states.begin(), states.end(), [newState](shared_ptr<CharacterState> state)
                               { return state->name == newState->name; });

  if (stateIterator != states.end())
    RemoveState(stateIterator, true);

  // Add it
  states.push_back(newState);
}
