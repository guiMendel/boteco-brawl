#include "PlayerInput.h"
#include "FallOffDeath.h"

using namespace std;

PlayerInput::PlayerInput(GameObject &associatedObject)
    : Component(associatedObject), weakMovement(gameObject.RequireComponent<Movement>())
{
  auto reset = [this]()
  {
    SetDirection({0, 0});
  };

  gameObject.RequireComponent<FallOffDeath>()->OnDeath.AddListener("reset-movement-direction", reset);
}

void PlayerInput::SetDirection(Vector2 direction)
{
  if (direction == currentDirection)
    return;

  // Keep last direction
  auto lastDirection = currentDirection;

  // Update the new one
  currentDirection = direction;

  // Raise movement if x changed
  if (GetSign(lastDirection.x, 0) != GetSign(direction.x, 0))
    OnMoveDirection.Invoke(GetCurrentMoveDirection());

  // Raise fast fall if y changed
  if (GetSign(lastDirection.y, 0) != GetSign(direction.y, 0))
  {
    if (direction.y > 0)
      OnFastFall.Invoke();

    else
      OnFastFallStop.Invoke();
  }
}

void PlayerInput::CancelMoveDirection(float direction)
{
  if ((currentDirection.x <= 0 && direction >= 0) || (currentDirection.x >= 0 && direction <= 0))
    return;

  SetDirection({0, currentDirection.y});
}

float PlayerInput::GetCurrentMoveDirection() const { return GetSign(currentDirection.x, 0); }

void PlayerInput::Attack()
{
  LOCK(weakMovement, movement);

  // When grounded
  if (movement->IsGrounded())
  {
    // Neutral if direction is small
    if (currentDirection.SqrMagnitude() < 0.1)
      OnAttackNeutral.Invoke();

    else if (abs(currentDirection.x) >= abs(currentDirection.y))
      OnAttackHorizontal.Invoke();

    else if (currentDirection.y < 0)
      OnAttackUp.Invoke();
  }

  // When in air
  else
  {
    // Horizontal if direction is small
    if (currentDirection.SqrMagnitude() < 0.1)
      OnAirHorizontal.Invoke();

    // Check if vertical
    else if (abs(currentDirection.x) < abs(currentDirection.y))
    {
      if (currentDirection.y < 0)
        OnAirUp.Invoke();

      else
        OnAirDown.Invoke();
    }

    else
      OnAirHorizontal.Invoke();
  }
}

void PlayerInput::Special()
{
  // Neutral if direction is small
  if (currentDirection.SqrMagnitude() < 0.1)
    OnSpecialNeutral.Invoke();

  else if (abs(currentDirection.x) >= abs(currentDirection.y))
    OnSpecialHorizontal.Invoke();
}
