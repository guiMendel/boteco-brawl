#ifndef __PLAYER_MANAGER__
#define __PLAYER_MANAGER__

#include "WorldObject.h"
#include "WorldComponent.h"
#include "Player.h"
#include "Event.h"
#include <memory>
#include <vector>

#define PLAYER_OBJECT_NAME "Player"

#define PLAYER_1_COLOR Color(41, 164, 17)
#define PLAYER_2_COLOR Color(243, 134, 14)
#define PLAYER_3_COLOR Color(1, 17, 172)
#define PLAYER_4_COLOR Color(204, 1, 1)

// Handles human players connected to the game
class PlayerManager : public WorldComponent
{
public:
  // Raised when a player starts searching for a new controller
  EventI<std::shared_ptr<Player>> OnPlayerSearchForController;

  // Colors of each player
  static const Color playerColors[];

  PlayerManager(GameObject &associatedObject);

  virtual ~PlayerManager() {}

  std::shared_ptr<Player> GetPlayer(int playerId) const;
  std::shared_ptr<Player> RequirePlayer(int playerId) const;

  std::vector<std::shared_ptr<Player>> GetPlayers() const;
  std::shared_ptr<Player> GetMainPlayer() const;

  // Sets which player should be the main
  void SetMainPlayer(std::shared_ptr<Player> player);

  // Creates a new player
  template <class T, typename... Args>
  std::shared_ptr<T> AddNewPlayer(Args &&...args)
  {
    // Get current players
    auto players = GetPlayers();

    // Create and pick this player's color
    auto newPlayer = worldObject.CreateChild(PLAYER_OBJECT_NAME)->AddComponent<T>(*this, playerColors[players.size()], std::forward<Args>(args)...);

    // If this is the only player, set it as main
    if (GetPlayers().size() == 1)
      SetMainPlayer(newPlayer);

    return newPlayer;
  }

private:
  // Component id of main player worldObject
  // -1 if no player
  int mainPlayerId{-1};
};

#endif