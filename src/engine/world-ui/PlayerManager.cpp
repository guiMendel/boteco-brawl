#include "PlayerManager.h"
#include <queue>

using namespace std;

// Set player colors
const Color PlayerManager::playerColors[]{PLAYER_1_COLOR, PLAYER_2_COLOR, PLAYER_3_COLOR, PLAYER_4_COLOR};

PlayerManager::PlayerManager(GameObject &associatedObject)
    : WorldComponent(associatedObject) {}

void PlayerManager::Awake() { UseCurrentControllers(); }

void PlayerManager::SetMainPlayer(shared_ptr<Player> player) { mainPlayerId = player->PlayerId(); }

vector<shared_ptr<Player>> PlayerManager::GetPlayers() const
{
  return worldObject.GetComponentsInChildren<Player>();
}

shared_ptr<Player> PlayerManager::GetMainPlayer() const
{
  Assert(mainPlayerId >= 0, "Tried to get main player when it's never been set");

  return GetScene()->RequireWorldObject(mainPlayerId)->RequireComponent<Player>();
}

shared_ptr<Player> PlayerManager::GetPlayer(int playerId) const
{
  for (auto player : GetPlayers())
    if (player->PlayerId() == playerId)
      return player;

  return nullptr;
}

shared_ptr<Player> PlayerManager::RequirePlayer(int playerId) const
{
  auto player = GetPlayer(playerId);

  Assert(player != nullptr, "Couldn't find required player with id " + to_string(playerId));

  return player;
}

void PlayerManager::UseCurrentControllers()
{
  // Get players looking for controllers
  queue<shared_ptr<Player>> availablePlayers;

  for (auto player : GetPlayers())
    if (player->SearchingForController())
      availablePlayers.push(player);

  for (auto controller : inputManager.GetControllers())
  {
    controller->RegisterPlayerManager(RequirePointerCast<PlayerManager>(GetShared()));

    if (controller->SearchingForPlayer() && availablePlayers.size() > 0)
    {
      controller->AssociateToPlayer(availablePlayers.front());
      availablePlayers.pop();
    }
  }
}
