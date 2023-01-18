#include "KeyboardInput.h"
#include "Movement.h"
#include "InputManager.h"

KeyboardInput::KeyboardInput(GameObject &associatedObject) : PlayerInput(associatedObject) {}

Vector2 KeyboardInput::GetInputDirection() const
{
  Vector2 inputDirection = Vector2::Zero();

  if (inputManager.IsKeyDown(SDLK_a))
    inputDirection.x = -1;

  if (inputManager.IsKeyDown(SDLK_d))
  {
    // If left is also pressed, give it preference if it's newer
    if (inputDirection.x != -1 || inputManager.KeyStateLength(SDLK_a) >= inputManager.KeyStateLength(SDLK_d))
      inputDirection.x = 1;
  }

  if (inputManager.IsKeyDown(SDLK_w))
    inputDirection.y = -1;

  if (inputManager.IsKeyDown(SDLK_s))
  {
    // If up is also pressed, give it preference if it's newer
    if (inputDirection.y != -1 || inputManager.KeyStateLength(SDLK_w) >= inputManager.KeyStateLength(SDLK_s))
      inputDirection.y = 1;
  }

  return inputDirection.Normalized();
}

void KeyboardInput::Update(float)
{
  // Get this frame's input direction
  SetDirection(GetInputDirection());

  // Detect jump
  if (inputManager.KeyPress(SDLK_SPACE))
    OnJump.Invoke();

  // Detect dash input
  if (inputManager.KeyPress(SDLK_k))
    OnDash.Invoke(currentDirection);

  // Detect attack
  if (inputManager.KeyPress(SDLK_j))
    Attack();

  else if (inputManager.KeyRelease(SDLK_j))
    OnReleaseAttack.Invoke();

  // Detect special
  if (inputManager.KeyPress(SDLK_l))
    Special();

  else if (inputManager.KeyRelease(SDLK_l))
    OnReleaseSpecial.Invoke();
}
