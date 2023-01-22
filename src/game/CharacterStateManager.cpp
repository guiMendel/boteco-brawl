#include "CharacterStateManager.h"
#include "Action.h"
#include <algorithm>
#include <typeinfo>

#define CONTROL_RECOVER_TIMER "control-recover"

using namespace std;

const float CharacterStateManager::dangerousFlySpeed{15};

const float CharacterStateManager::maxActionDelay{1};

CharacterStateManager::CharacterStateManager(GameObject &associatedObject)
    : Component(associatedObject) {}

void CharacterStateManager::Update(float deltaTime)
{
  HandleStateExpiration();

  HandleQueuedAction(deltaTime);

  // Raise control change
  bool frameControl = HasControl();

  if (lastFrameControl != frameControl)
    OnControlChange.Invoke(frameControl);

  lastFrameControl = frameControl;
}

void CharacterStateManager::Awake()
{
  weakBody = gameObject.RequireComponent<Rigidbody>();
}

void CharacterStateManager::HandleQueuedAction(float deltaTime)
{
  if (queuedActionTTL <= 0)
    return;

  // Maybe perform the queued action
  if (queuedAction != nullptr && CanPerform(queuedAction))
  {
    Perform(queuedAction);
    ResetQueue();
    return;
  }

  // Discount queue time
  queuedActionTTL -= deltaTime;

  // If it timed out, discard it
  if (queuedActionTTL <= 0)
    ResetQueue();
}

const list<shared_ptr<CharacterState>> CharacterStateManager::GetStates() const { return states; }
void CharacterStateManager::SetState(shared_ptr<CharacterState> newState, unordered_set<string> keepStates)
{
  // Add it
  AddState(newState);

  // Remove other states
  // Make sure not to remove it right away
  keepStates.insert(newState->name);

  RemoveStatesNotIn(keepStates, true);
}

void CharacterStateManager::RemoveStatesNotIn(std::unordered_set<std::string> keepStates, bool interruption)
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
      stateIterator = RemoveState(stateIterator, interruption);
  }
}

void CharacterStateManager::ResetQueue()
{
  queuedAction = nullptr;
  queuedActionTTL = 0;
}

void CharacterStateManager::QueueAction(std::shared_ptr<Action> action)
{
  queuedAction = action;

  // Reset time
  queuedActionTTL = maxActionDelay;
}

bool CharacterStateManager::CanPerform(std::shared_ptr<Action> action)
{
  // Check if this state blocks the incoming action
  auto stateBlocksAction = [action](shared_ptr<CharacterState> state)
  {
    return (
        // It must have higher priority
        state->priority > action->GetPriority() &&
        // And not be a friend of the action
        !action->IsFriend(state) &&
        // And either it's parent action is different from it, or the state is not yet open to sequence
        (*state->parentAction != *action || state->openToSequence == false));
  };

  // If none of the states blocks this action, it can be performed
  return action->IsValid(gameObject) && any_of(states.begin(), states.end(), stateBlocksAction) == false;
}

void CharacterStateManager::SetActionSequenceIndex(shared_ptr<Action> action)
{
  // For each state
  for (auto state : states)
  {
    // Check if this state's parent action is the same
    // cout << "Comparing actions " << typeid(*state->parentAction).name() << " and " << typeid(*action).name() << ": " << (*state->parentAction == *action) << endl;
    if (*state->parentAction == *action)
    {
      // Check if the sequence index for this action will be higher than the current one
      action->sequenceIndex = max(action->sequenceIndex, state->parentAction->sequenceIndex + 1);
    }
  }
}

void CharacterStateManager::Perform(shared_ptr<Action> action, bool canDelay)
{
  // cout << "States: ";
  // for (auto state : states)
  //   cout << state->name << " ";
  // cout << endl;

  // Set incoming action's sequence index
  SetActionSequenceIndex(action);

  if (CanPerform(action) == false)
  {
    // Push to queue if possible
    if (canDelay)
      QueueAction(action);

    // Otherwise, empty queue
    else
      ResetQueue();

    return;
  }

  // This action's yielded state
  shared_ptr<CharacterState> newState = action->NextState(action);

  // Set new state
  if (newState != nullptr)
    SetState(newState, action->GetFriendStates());

  // If no new state, simply remove those not in the list
  else
    RemoveStatesNotIn(action->GetFriendStates(), true);

  // Trigger this action
  action->Trigger(gameObject, newState);
}

void CharacterStateManager::RemoveState(unsigned id, bool interruption)
{
  // Find this state
  auto stateIterator = find_if(states.begin(), states.end(), [id](shared_ptr<CharacterState> state)
                               { return state->id == id; });

  //  If it's not there, stop
  if (stateIterator == states.end())
    return;

  // Remove it
  RemoveState(stateIterator, interruption);
}

auto CharacterStateManager::RemoveState(decltype(states)::iterator stateIterator, bool interruption, bool ignoreIdleEvent) -> decltype(states)::iterator
{
  auto state = *stateIterator;

  // Call it's stop callback if necessary
  if (state->parentAction != nullptr)
    state->parentAction->StopHook(gameObject, state);

  // Announce interruption
  if (interruption)
    OnInterruptState.Invoke(state);

  // Remove it
  auto newIterator = states.erase(stateIterator);

  // Check for raise
  if (ignoreIdleEvent == false && states.empty())
    OnEnterIdle.Invoke();

  return newIterator;
}

void CharacterStateManager::AddState(std::shared_ptr<CharacterState> newState)
{
  // If it's already there, remove it first
  auto stateIterator = find_if(states.begin(), states.end(), [newState](shared_ptr<CharacterState> state)
                               { return state->name == newState->name; });

  if (stateIterator != states.end())
    RemoveState(stateIterator, true, true);

  // Add it
  states.push_back(newState);
}

bool CharacterStateManager::HasState(std::string stateName)
{
  return find_if(states.begin(), states.end(), [stateName](shared_ptr<CharacterState> state)
                 { return state->name == stateName; }) != states.end();
}

shared_ptr<Action> CharacterStateManager::GetQueuedAction() const { return queuedAction; }

bool CharacterStateManager::HasControl() const
{
  // If any state loses control, no control
  return all_of(states.begin(), states.end(), [](shared_ptr<CharacterState> state)
                { return state->losesControl == false; });
}

void CharacterStateManager::HandleStateExpiration()
{
  auto shared = dynamic_pointer_cast<CharacterStateManager>(GetShared());
  
  auto stateIterator = states.begin();
  while (stateIterator != states.end())
  {
    auto state = *stateIterator;

    // Remove it if requested
    if (state->RemoveRequested(shared))
      stateIterator = RemoveState(stateIterator, false);

    // Otherwise, cary on
    else
      stateIterator++;
  }
}
