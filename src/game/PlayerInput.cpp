#include "PlayerInput.h"

PlayerInput::PlayerInput(GameObject &associatedObject) : Component(associatedObject) {}

void PlayerInput::SetDirection(float direction)
{
  OnMoveDirection.Invoke(direction);
  currentDirection = direction;
}

void PlayerInput::CancelDirection(float direction)
{
  if ((currentDirection <= 0 && direction >= 0) || (currentDirection >= 0 && direction <= 0))
    return;

  currentDirection = 0;
  OnMoveDirection.Invoke(0);
}

float PlayerInput::GetCurrentMoveDirection() const { return currentDirection; }