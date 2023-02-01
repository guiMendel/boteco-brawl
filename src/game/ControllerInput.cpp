#include "ControllerInput.h"
#include "CharacterController.h"
#include "Movement.h"
#include "InputManager.h"

#include <string>

using namespace std;

ControllerInput::ControllerInput(GameObject &associatedObject)
    : PlayerInput(associatedObject),
      weakPlayer(worldObject.RequireComponent<CharacterController>()->GetPlayer())
{
  LOCK(weakPlayer, player);

  MESSAGE << "Using controller for player " << player->PlayerId() << endl;

  // Make sure this player is using a controller
  player->UseController();
}

void ControllerInput::HandleAnalogMovement(Vector2 newDirection)
{
  // Normalize direction
  SetDirection(newDirection.Normalized());
}

void ControllerInput::HandleButtonPress(SDL_GameControllerButton button)
{
  // Detect jump
  if (button == SDL_CONTROLLER_BUTTON_A || button == SDL_CONTROLLER_BUTTON_B)
    OnJump.Invoke();

  // Detect dash
  if (button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
    OnDash.Invoke(currentDirection);

  // Detect attack
  if (button == SDL_CONTROLLER_BUTTON_X)
    Attack();

  // Detect special
  if (button == SDL_CONTROLLER_BUTTON_Y)
    Special();
}

void ControllerInput::HandleButtonRelease(SDL_GameControllerButton button)
{
  // Detect attack
  if (button == SDL_CONTROLLER_BUTTON_X)
    OnReleaseAttack.Invoke();

  // Detect special
  if (button == SDL_CONTROLLER_BUTTON_Y)
    OnReleaseSpecial.Invoke();
}

void ControllerInput::Start()
{
  LOCK(weakPlayer, player);

  // This component callback identifier
  string callbackIdentifier = "controller-input-player-" + to_string(player->PlayerId());

  // Subscribe to analog movement
  inputManager.OnControllerLeftAnalog.AddListener(callbackIdentifier, [this](Vector2 direction, int targetId)
                                                  { if (targetId == GetAssociatedControllerId()) HandleAnalogMovement(direction); });

  // Subscribe to analog buttons
  inputManager.OnControllerButtonPress.AddListener(callbackIdentifier, [this](SDL_GameControllerButton button, int targetId)
                                                   { if (targetId == GetAssociatedControllerId()) HandleButtonPress(button); });

  inputManager.OnControllerButtonRelease.AddListener(callbackIdentifier, [this](SDL_GameControllerButton button, int targetId)
                                                     { if (targetId == GetAssociatedControllerId()) HandleButtonRelease(button); });
}

int ControllerInput::GetAssociatedControllerId() const
{
  LOCK(weakPlayer, player);

  auto controller = player->GetController();

  return controller == nullptr ? -1 : controller->instanceId;
}
