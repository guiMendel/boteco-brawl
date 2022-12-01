#include "PlayerController.h"
#include "Movement.h"
#include "InputManager.h"

PlayerController::PlayerController(GameObject &associatedObject)
    : Component(associatedObject), movement(*gameObject.RequireComponent<Movement>()) {}

void PlayerController::Update([[maybe_unused]] float deltaTime)
{
  // Detect idle
  if (inputManager.KeyRelease(SDLK_d))
    CancelDirection(1);

  if (inputManager.KeyRelease(SDLK_a))
    CancelDirection(-1);

  // Detect keyboard movement keys
  if (inputManager.KeyPress(SDLK_d))
    SetDirection(1);

  else if (inputManager.KeyPress(SDLK_a))
    SetDirection(-1);
}

void PlayerController::SetDirection(float direction)
{
  movement.SetDirection(direction);
  currentDirection = direction;
}

void PlayerController::CancelDirection(float direction)
{
  if ((currentDirection <= 0 && direction >= 0) || (currentDirection >= 0 && direction <= 0))
    return;

  movement.SetDirection(0);
  currentDirection = 0;
}