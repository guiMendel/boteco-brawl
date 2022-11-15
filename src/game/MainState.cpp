#include "MainState.h"
#include "GameData.h"
#include "Recipes.h"
#include "Camera.h"
#include "Tilemap.h"

using namespace std;

void MainState::InitializeObjects()
{
  // Add a background
  CreateObject("Background", Recipes::Background("./assets/image/boteco.jpg"));

  // Play music
  music.Play("./assets/music/main.mp3");
}
