#ifndef __CONTROLLER_INPUT__
#define __CONTROLLER_INPUT__

#include "PlayerInput.h"

class ControllerInput : public PlayerInput
{
public:
  ControllerInput(GameObject &associatedObject, int bindInstanceId);
  virtual ~ControllerInput() {}

  void Start() override;

private:
  // Handles analog movement
  void HandleAnalogMovement(Vector2 newDirection);

  void HandleAnalogButton(SDL_GameControllerButton button);

  // To which controller instance id this component is bound
  int controllerInstanceId;

  // Last left analog input
  Vector2 analogDirection;
};

#endif