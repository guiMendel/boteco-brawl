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
  Player(GameObject &associatedObject, PlayerManager &manager);

  virtual ~Player() {}

  // Whether this player is currently searching for an available controller to associate to
  bool SearchingForController() const;

  // Associate a controller to this player
  void AssociateController(std::shared_ptr<ControllerDevice> newDevice);

  // Configures to use a controller and search for one when not in possession of
  void UseController();

  // Starts searching for a new controller
  void SearchForController();

  // Erases association from controller
  void LoseController();

  // Get the associated control (if any)
  std::shared_ptr<ControllerDevice> GetController() const;

  // Whether this is the main player
  bool IsMain() const;

  // Gets id used to identify this player
  int PlayerId() const;

private:
  // Controller associated to this player
  std::weak_ptr<ControllerDevice> weakController;

  // Whether this player should have an associated controller
  bool usingController{false};

  // Reference to player manager
  PlayerManager &playerManager;
};

#endif