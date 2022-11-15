#include <iostream>
#include "Game.h"
// #include "test.h"

using namespace std;

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
  // Get game instance & run
  try
  {
    Game &gameInstance = Game::GetInstance();

    gameInstance.Start();
  }
  // Catch any runtime errors
  catch (const runtime_error &error)
  {
    cerr << "=> ERROR: " << error.what() << endl;
  }

  return 0;
}