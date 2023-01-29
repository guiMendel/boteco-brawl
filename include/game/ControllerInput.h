#ifndef __CONTROLLER_INPUT__
#define __CONTROLLER_INPUT__

#include "PlayerInput.h"
#include "Player.h"
#include <memory>

class ControllerInput : public PlayerInput
{
public:
  ControllerInput(GameObject &associatedObject);
  virtual ~ControllerInput() {}

  void Start() override;

  // Get controller instance id associated to this controller input's player
  int GetAssociatedControllerId() const;

private:
  // Handles analog movement
  void HandleAnalogMovement(Vector2 newDirection);

  void HandleButtonPress(SDL_GameControllerButton button);
  void HandleButtonRelease(SDL_GameControllerButton button);

  // To which player this controller input is bound
  const std::weak_ptr<Player> weakPlayer;
};

#endif