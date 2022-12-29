#include "MainState.h"
#include "GameData.h"
#include "ObjectRecipes.h"
#include "PlayerManager.h"
#include "ParticleFX.h"
#include "Camera.h"

using namespace std;

void MainState::LoadAssets()
{
}

void MainState::OnUpdate(float)
{
  // auto thing = FindObject("Thing");
  // cout << "Thing at: " << thing->GetPosition() << endl;
}

void MainState::InitializeObjects()
{
  // Add player manager
  auto playerManager = CreateObject("PlayerManager", ObjectRecipes::SingleComponent<PlayerManager>(true))->RequireComponent<PlayerManager>();

  // // Add particle fx
  CreateObject("ParticleFX", ObjectRecipes::SingleComponent<ParticleFX>());

  // // Add a background
  CreateObject("Background", ObjectRecipes::Background("./assets/image/boteco.jpg"));

  // // Add player
  // auto player1 = CreateObject("Character", ObjectRecipes::Character(playerManager->GetMainPlayer()), Vector2(-4, -10))->GetComponent<Rigidbody>();

  // // auto player2 = CreateObject("Player2", ObjectRecipes::Character(), Vector2(4, -2))->GetComponent<Rigidbody>();

  // // TODO: fix theses platforms' destructors being called on start of game for some reason
  CreateObject("Ground", ObjectRecipes::Platform({20, 2}), Vector2(0, 4));

  CreateObject("WallLeft", ObjectRecipes::Platform({1, 5}), Vector2(-5, 0));
  CreateObject("WallRight", ObjectRecipes::Platform({1, 5}), Vector2(5, 0));

  auto thing = CreateObject("Thing", ObjectRecipes::Platform({1, 1}, false), Vector2(0, 0));

  auto thingBody = thing->GetComponent<Rigidbody>();
  thingBody->continuousCollisions = true;
  // // thingBody->velocity += Vector2({-1, 0});

  // player1->velocity = Vector2(2, 0);
  // player2->velocity = Vector2(-2, 1);

  // Play music
  // music.Play("./assets/music/main.mp3");
}
