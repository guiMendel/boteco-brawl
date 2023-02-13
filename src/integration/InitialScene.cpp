#include "Game.h"
#include "ArenaScene.h"

using namespace std;

shared_ptr<GameScene> Game::GetInitialScene() const
{
  return make_unique<ArenaScene>();
}