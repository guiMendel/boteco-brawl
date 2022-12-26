#include "ControllerInput.h"
#include "Movement.h"
#include "InputManager.h"

#include <string>

using namespace std;

ControllerInput::ControllerInput(GameObject &associatedObject, int bindInstanceId)
    : PlayerInput(associatedObject), controllerInstanceId(bindInstanceId) {}

void ControllerInput::HandleAnalogMovement(Vector2 newDirection)
{
  // Move on X axis
  if (newDirection.x != analogDirection.x)
    SetDirection(newDirection.x);

  // Fast fall start
  if (newDirection.y > 0 && analogDirection.y <= 0)
    OnFastFall.Invoke();

  // Fast fall stop
  if (newDirection.y <= 0 && analogDirection.y > 0)
    OnFastFallStop.Invoke();

  // Store this direction
  analogDirection = newDirection;
}

void ControllerInput::HandleAnalogButton(SDL_GameControllerButton button)
{
  // Detect jump
  if (button == SDL_CONTROLLER_BUTTON_A || button == SDL_CONTROLLER_BUTTON_B)
    OnJump.Invoke();

  // Detect neutral attack
  if (button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
    OnNeutralAttack.Invoke();

  // Detect dash
  if (button == SDL_CONTROLLER_BUTTON_X || button == SDL_CONTROLLER_BUTTON_Y)
    OnDash.Invoke(analogDirection);
}

void ControllerInput::Start()
{
  // This component callback identifier
  string callbackIdentifier = "controller-input-" + to_string(controllerInstanceId);

  // Subscribe to analog movement
  inputManager.OnControllerLeftAnalog.AddListener(callbackIdentifier, [this](Vector2 direction, int targetId)
                                                  { if (targetId == controllerInstanceId) HandleAnalogMovement(direction); });

  // Subscribe to analog buttons
  inputManager.OnControllerButtonPress.AddListener(callbackIdentifier, [this](SDL_GameControllerButton button, int targetId)
                                                   { if (targetId == controllerInstanceId) HandleAnalogButton(button); });
}
