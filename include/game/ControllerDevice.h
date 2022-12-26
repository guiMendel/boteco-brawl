#ifndef __CONTROLLER_DEVICE__
#define __CONTROLLER_DEVICE__

#include "SDL_gamecontroller.h"
#include <memory>

class Player;

// Class that is responsible for a connected game controller device
class ControllerDevice
{
public:
  // Requires an index of a connected controller
  ControllerDevice(int controllerIndex);

private:
  // Associates this controller to a player
  void AssociateToPlayer(std::shared_ptr<Player> player);

  // Which player is associated to this controller
  std::weak_ptr<Player> weakAssociationPlayer;

  // Stores the instance ID of this controller
  const int instanceId;

  // Stores the game controller SDL struct of this controller
  const std::unique_ptr<SDL_GameController> controllerStruct;
};

#endif