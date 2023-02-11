#include "PlayerManager.h"

using namespace std;

// Set player colors
const Color PlayerManager::playerColors[]{Color(41, 164, 17), Color(243, 134, 14), Color(1, 17, 172), Color(204, 1, 1)};

PlayerManager::PlayerManager(GameObject &associatedObject)
    : WorldComponent(associatedObject) {}

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
