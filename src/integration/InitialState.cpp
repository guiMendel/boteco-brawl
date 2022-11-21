#include "Game.h"
#include "MainState.h"

using namespace std;

unique_ptr<GameState> Game::GetInitialState() const
{
  return make_unique<MainState>();
}