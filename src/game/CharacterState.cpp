#include "CharacterState.h"
#include "Action.h"

using namespace std;

unsigned CharacterState::idGenerator{0};

CharacterState::CharacterState(string name, int priority, shared_ptr<Action> parentAction)
    : name(name), priority(priority), parentAction(parentAction)
{
  // If action input was already released, register it
  if (parentAction != nullptr)
    actionInputReleased = parentAction->actionInputAlreadyReleased;
}

bool CharacterState::ActionInputReleased() const { return actionInputReleased; }

void CharacterState::ReleaseActionInput()
{
  actionInputReleased = true;
  OnActionInputRelease.Invoke();
}

bool CharacterState::RemoveRequested(std::shared_ptr<CharacterStateManager> stateManager)
{
  if (removeCondition == nullptr)
    return false;

  return removeCondition(stateManager);
}
