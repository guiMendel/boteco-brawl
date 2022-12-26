#include "PlayerManager.h"

#define PLAYER_OBJECT_NAME "Player"

using namespace std;

PlayerManager::PlayerManager(GameObject &associatedObject)
    : Component(associatedObject)
{
  // Add main player
  AddNewPlayer();
}

shared_ptr<Player> PlayerManager::AddNewPlayer()
{
  auto newPlayer = gameObject.CreateChild(PLAYER_OBJECT_NAME)->AddComponent<Player>(*this);

  // If this is the only player, set it as main
  auto players = GetPlayers();

  if (players.size() == 1)
    SetMainPlayer(newPlayer);

  return newPlayer;
}

void PlayerManager::SetMainPlayer(shared_ptr<Player> player) { mainPlayerId = player->PlayerId(); }

vector<shared_ptr<Player>> PlayerManager::GetPlayers() const
{
  return gameObject.GetComponentsInChildren<Player>();
}

shared_ptr<Player> PlayerManager::GetMainPlayer() const
{
  Assert(mainPlayerId >= 0, "Tried to get main player when it's never been set");

  auto object = GetState()->GetObject(mainPlayerId);

  Assert(object != nullptr, "Unexpectedly failed to find main player gameObject");

  return object->RequireComponent<Player>();
}
