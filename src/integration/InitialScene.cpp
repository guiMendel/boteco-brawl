#include "Game.h"
#include "MainScene.h"

using namespace std;

shared_ptr<GameScene> Game::GetInitialScene() const
{
  return make_unique<MainScene>();
}