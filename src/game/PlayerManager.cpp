#include "PlayerManager.h"

#define PLAYER_OBJECT_NAME "Player"

using namespace std;

// Set player colors
const Color PlayerManager::playerColors[]{Color::Green(), Color::Yellow(), Color::Blue(), Color::Red()};

PlayerManager::PlayerManager(WorldObject &associatedObject)
    : Component(associatedObject)
{
  // Add main player
  AddNewPlayer();
}

shared_ptr<Player> PlayerManager::AddNewPlayer()
{
  // Get current players
  auto players = GetPlayers();

  Assert(players.size() < 4, "Tried adding more players than allowed");

  // Create and pick this player's color
  auto newPlayer = worldObject.CreateChild(PLAYER_OBJECT_NAME)->AddComponent<Player>(*this, playerColors[players.size()]);

  // If this is the only player, set it as main
  if (GetPlayers().size() == 1)
    SetMainPlayer(newPlayer);

  return newPlayer;
}

void PlayerManager::SetMainPlayer(shared_ptr<Player> player) { mainPlayerId = player->PlayerId(); }

vector<shared_ptr<Player>> PlayerManager::GetPlayers() const
{
  return worldObject.GetComponentsInChildren<Player>();
}

shared_ptr<Player> PlayerManager::GetMainPlayer() const
{
  Assert(mainPlayerId >= 0, "Tried to get main player when it's never been set");

  auto object = GetState()->GetObject(mainPlayerId);

  Assert(object != nullptr, "Unexpectedly failed to find main player worldObject");

  return object->RequireComponent<Player>();
}
