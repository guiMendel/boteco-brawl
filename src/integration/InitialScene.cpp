#include "Game.h"
#include "MenuScene.h"

using namespace std;

shared_ptr<GameScene> Game::GetInitialScene() const
{
  return make_unique<MenuScene>();
}