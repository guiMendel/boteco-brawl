#include "Game.h"
#include "SplashScene.h"

using namespace std;

shared_ptr<GameScene> Game::GetInitialScene() const
{
  return make_unique<SplashScene>();
}