#include "PlayerInput.h"

using namespace std;

PlayerInput::PlayerInput(GameObject &associatedObject) : Component(associatedObject) {}

void PlayerInput::SetDirection(float direction)
{
  // "Normalize" direction
  if (direction != 0)
    direction = direction > 0 ? 1 : -1;

  currentDirection = direction;
  OnMoveDirection.Invoke(direction);
}

void PlayerInput::CancelDirection(float direction)
{
  if ((currentDirection <= 0 && direction >= 0) || (currentDirection >= 0 && direction <= 0))
    return;

  currentDirection = 0;
  OnMoveDirection.Invoke(0);
}

float PlayerInput::GetCurrentMoveDirection() const { return currentDirection; }