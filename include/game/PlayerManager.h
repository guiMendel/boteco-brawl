#ifndef __PLAYER_MANAGER__
#define __PLAYER_MANAGER__

#include "GameObject.h"
#include "Component.h"
#include "Player.h"
#include "Event.h"
#include <memory>
#include <vector>

class PlayerManager : public Component
{
public:
  // Raised when a player starts searching for a new controller
  EventI<std::shared_ptr<Player>> OnPlayerSearchForController;

  PlayerManager(GameObject &associatedObject);

  virtual ~PlayerManager() {}

  std::vector<std::shared_ptr<Player>> GetPlayers() const;

  // Sets which player should be the main
  void SetMainPlayer(int newId);

  // Creates a new player
  void AddNewPlayer();

private:
  // Component id of main player
  int mainPlayerId;
};

#endif