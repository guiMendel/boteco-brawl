#include "Game.h"
#include "CharacterSelectScene.h"

using namespace std;

shared_ptr<GameScene> Game::GetInitialScene() const
{
  return make_unique<CharacterSelectScene>();
}