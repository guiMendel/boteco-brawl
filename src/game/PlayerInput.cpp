#include "PlayerInput.h"
#include "Movement.h"
#include "InputManager.h"

PlayerInput::PlayerInput(GameObject &associatedObject) : Component(associatedObject) {}

Vector2 PlayerInput::GetInputDirection() const
{
  Vector2 inputDirection = Vector2::Zero();

  if (inputManager.IsKeyDown(SDLK_a))
    inputDirection.x -= 1;

  if (inputManager.IsKeyDown(SDLK_d))
    inputDirection.x += 1;

  if (inputManager.IsKeyDown(SDLK_w))
    inputDirection.y -= 1;

  if (inputManager.IsKeyDown(SDLK_s))
    inputDirection.y += 1;

  return inputDirection;
}

void PlayerInput::Update([[maybe_unused]] float deltaTime)
{
  // Get this frame's input direction
  Vector2 inputDirection = GetInputDirection();

  // Detect idle
  if (inputManager.KeyRelease(SDLK_d))
  {
    if (inputManager.IsKeyDown(SDLK_a))
      SetDirection(-1);
    else
      CancelDirection(1);
  }

  if (inputManager.KeyRelease(SDLK_a))
  {
    if (inputManager.IsKeyDown(SDLK_d))
      SetDirection(1);
    else
      CancelDirection(-1);
  }

  // Detect keyboard movement keys
  if (inputManager.KeyPress(SDLK_d))
    SetDirection(1);

  else if (inputManager.KeyPress(SDLK_a))
    SetDirection(-1);

  // Detect jump
  if (inputManager.KeyPress(SDLK_SPACE))
    OnJump.Invoke();

  // Detect fall acceleration
  if (inputManager.KeyPress(SDLK_s))
    OnFastFall.Invoke();

  else if (inputManager.KeyRelease(SDLK_s))
    OnFastFallStop.Invoke();

  if (inputManager.KeyPress(SDLK_j))
    OnNeutralAttack.Invoke();

  // Detect dash input
  if (inputManager.KeyPress(SDLK_k))
    OnDash.Invoke(inputDirection);
}

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