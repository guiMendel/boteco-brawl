#include "MainState.h"
#include "GameData.h"
#include "ObjectRecipes.h"
#include "Camera.h"

using namespace std;

void MainState::LoadAssets()
{
}

void MainState::InitializeObjects()
{
  // Add a background
  CreateObject("Background", ObjectRecipes::Background("./assets/image/boteco.jpg"));

  // Add player
  auto player1 = CreateObject("Player", ObjectRecipes::Character(), Vector2(-4, -10))->GetComponent<Rigidbody>();

  // auto player2 = CreateObject("Player2", ObjectRecipes::Character(), Vector2(4, -2))->GetComponent<Rigidbody>();

  CreateObject("Ground", ObjectRecipes::Platform({20, 2}), Vector2(0, 4));

  CreateObject("WallLeft", ObjectRecipes::Platform({1, 5}), Vector2(-5, 0));
  CreateObject("WallRight", ObjectRecipes::Platform({1, 5}), Vector2(5, 0));

  // auto thing = CreateObject("Thing", ObjectRecipes::Platform({1, 1}, false), Vector2(2, 0));

  // auto thingBody = thing->GetComponent<Rigidbody>();
  // thingBody->velocity += Vector2({-1, 0});

  player1->velocity = Vector2(2, 0);
  // player2->velocity = Vector2(-2, 1);

  // Play music
  // music.Play("./assets/music/main.mp3");
}
