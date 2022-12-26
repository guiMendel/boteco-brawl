#ifndef __CONTROLLER_INPUT__
#define __CONTROLLER_INPUT__

#include "PlayerInput.h"
#include "Player.h"
#include <memory>

class ControllerInput : public PlayerInput
{
public:
  ControllerInput(GameObject &associatedObject, std::shared_ptr<Player> player);
  virtual ~ControllerInput() {}

  void Start() override;

  // Get player associated to this controller input
  const std::shared_ptr<Player> GetPlayer() const;

  // Get controller instance id associated to this controller input's player
  int GetAssociatedControllerId() const;

private:
  // Handles analog movement
  void HandleAnalogMovement(Vector2 newDirection);

  void HandleAnalogButton(SDL_GameControllerButton button);

  // To which player this controller input is bound
  const std::weak_ptr<Player> weakPlayer;

  // Last left analog input
  Vector2 analogDirection;
};

#endif