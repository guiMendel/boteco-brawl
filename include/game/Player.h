#ifndef __PLAYER__
#define __PLAYER__

#include "GameObject.h"
#include "Component.h"
#include "ControllerDevice.h"

class PlayerManager;

// Defines a player that is connected to the game
class Player : public Component
{
public:
  Player(GameObject &associatedObject);

  virtual ~Player() {}

  // Whether this player is currently searching for an available controller to associate to
  bool SearchingForController() const;

  // Associate a controller to this player
  void AssociateController(std::shared_ptr<ControllerDevice> newDevice);

  // Erases association from controller
  void LoseController();

  // Whether this player has keyboard control
  bool HasKeyboardControl() const;

private:
  // Controller associated to this player
  std::weak_ptr<ControllerDevice> controllerDevice;

  // Whether this player should have an associated controller
  bool usingController{false};

  // Reference to player manager
  PlayerManager &playerManager;
};

#endif