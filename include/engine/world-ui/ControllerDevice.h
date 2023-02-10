#ifndef __CONTROLLER_DEVICE__
#define __CONTROLLER_DEVICE__

#include "SDL_gamecontroller.h"
#include "Helper.h"
#include <memory>
#include <string>

class Player;
class PlayerManager;

// Thrown when opening a controller fails
class invalid_controller_error : std::runtime_error
{
public:
  invalid_controller_error(std::string message) : runtime_error(message) {}

  operator std::string() const { return what(); }
};

// Class that is responsible for a connected game controller device
class ControllerDevice
{
public:
  // Requires an index of a connected controller
  ControllerDevice(int controllerIndex);

  virtual ~ControllerDevice();

  // Gets shared pointer of this device
  std::shared_ptr<ControllerDevice> GetShared() const;

  // Get player associated to this controller (if any)
  std::shared_ptr<Player> GetPlayer() const;

  // Forget current player and searches for a new one
  void LosePlayer();

  // Gets instance id of this controller
  int GetId() const;

  // Checks all players for one to associate to
  void SearchPlayersForAssociation();

  // Associates this controller to a player
  void AssociateToPlayer(std::shared_ptr<Player> player);

private:
  // Check if should associate to this player
  void MaybeAssociateToPlayer(std::shared_ptr<Player> player);

  // Which player is associated to this controller
  std::weak_ptr<Player> weakAssociationPlayer;

  // Stores the game controller SDL struct of this controller
  const Helper::auto_unique_ptr<SDL_GameController> controllerStruct;

  // Stores the instance ID of this controller
  const int instanceId;
};

#endif