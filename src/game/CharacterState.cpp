#include "CharacterState.h"

using namespace std;

unsigned CharacterState::idGenerator{0};

CharacterState::CharacterState(string name, int priority, shared_ptr<Action> parentAction)
    : name(name), priority(priority), parentAction(parentAction) {}

bool CharacterState::ActionInputReleased() const { return actionInputReleased; }

void CharacterState::ReleaseActionInput()
{
  actionInputReleased = true;
  OnActionInputRelease.Invoke();
}
