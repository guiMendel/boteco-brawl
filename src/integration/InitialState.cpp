#include "Game.h"
#include "MainState.h"

using namespace std;

shared_ptr<GameState> Game::GetInitialState() const
{
  return make_unique<MainState>();
}