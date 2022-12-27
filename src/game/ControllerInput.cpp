#include "ControllerInput.h"
#include "Movement.h"
#include "InputManager.h"

#include <string>

using namespace std;

ControllerInput::ControllerInput(GameObject &associatedObject, std::shared_ptr<Player> player)
    : PlayerInput(associatedObject), weakPlayer(player)
{
  cout << "Using controller for player " << player->PlayerId() << endl;

  // Make sure this player is using a controller
  player->UseController();
}

void ControllerInput::HandleAnalogMovement(Vector2 newDirection)
{
  // Normalize direction
  newDirection = newDirection.Normalized();

  if (newDirection == analogDirection)
    return;

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

  // Detect dash
  if (button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
    OnDash.Invoke(analogDirection);

  // Detect neutral attack
  if (button == SDL_CONTROLLER_BUTTON_X || button == SDL_CONTROLLER_BUTTON_Y)
    OnNeutralAttack.Invoke();
}

void ControllerInput::Start()
{
  // This component callback identifier
  string callbackIdentifier = "controller-input-player-" + to_string(GetPlayer()->PlayerId());

  // Subscribe to analog movement
  inputManager.OnControllerLeftAnalog.AddListener(callbackIdentifier, [this](Vector2 direction, int targetId)
                                                  { if (targetId == GetAssociatedControllerId()) HandleAnalogMovement(direction); });

  // Subscribe to analog buttons
  inputManager.OnControllerButtonPress.AddListener(callbackIdentifier, [this](SDL_GameControllerButton button, int targetId)
                                                   { if (targetId == GetAssociatedControllerId()) HandleAnalogButton(button); });
}

// Get player associated to this controller input
const shared_ptr<Player> ControllerInput::GetPlayer() const
{
  Assert(weakPlayer.expired() == false, "ControllerInput unexpectedly lost pointer to it's associated player");

  return weakPlayer.lock();
}

int ControllerInput::GetAssociatedControllerId() const
{
  auto controller = GetPlayer()->GetController();

  return controller == nullptr ? -1 : controller->instanceId;
}
