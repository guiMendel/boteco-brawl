#include "PlayerInput.h"
#include "FallOffDeath.h"

using namespace std;

PlayerInput::PlayerInput(GameObject &associatedObject) : Component(associatedObject)
{
  auto reset = [this]()
  {
    SetDirection(0);
  };

  gameObject.RequireComponent<FallOffDeath>()->OnDeath.AddListener("reset-movement-direction", reset);
}

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

  SetDirection(0);
}

float PlayerInput::GetCurrentMoveDirection() const { return currentDirection; }