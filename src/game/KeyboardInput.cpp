#include "KeyboardInput.h"
#include "Movement.h"
#include "InputManager.h"

KeyboardInput::KeyboardInput(GameObject &associatedObject) : PlayerInput(associatedObject) {}

Vector2 KeyboardInput::GetInputDirection() const
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

  return inputDirection.Normalized();
}

void KeyboardInput::Update(float)
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

  // Detect attack
  if (inputManager.KeyPress(SDLK_j))
    OnNeutralAttack.Invoke();

  // Detect special
  if (inputManager.KeyPress(SDLK_l))
    OnNeutralSpecial.Invoke();

  // Detect dash input
  if (inputManager.KeyPress(SDLK_k))
    OnDash.Invoke(inputDirection);
}
