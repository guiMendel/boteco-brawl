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

  // Add player 2
  auto player2 = playerManager->AddNewPlayer();

  // Add particle fx
  CreateObject("ParticleFX", ObjectRecipes::SingleComponent<ParticleFX>());

  // Add a background
  CreateObject("Background", ObjectRecipes::Background("./assets/image/boteco.jpg"));

  // Add player
  auto character1 = CreateObject("Character", ObjectRecipes::Character(playerManager->GetMainPlayer()), Vector2(-3.5, -10))->GetComponent<Rigidbody>();

  auto character2 = CreateObject("Character2", ObjectRecipes::Character(player2), Vector2(3.5, -10))->GetComponent<Rigidbody>();

  CreateObject("Ground", ObjectRecipes::Platform({20, 2}), Vector2(0, 4));

  CreateObject("WallLeft", ObjectRecipes::Platform({1, 5}), Vector2(-7, 0));
  CreateObject("WallRight", ObjectRecipes::Platform({1, 5}), Vector2(7, 0));

  // auto thing = CreateObject("Thing", ObjectRecipes::Platform({1, 1}, false), Vector2(0, 0));

  // auto thingBody = thing->GetComponent<Rigidbody>();
  // thingBody->continuousCollisions = true;
  // thingBody->velocity += Vector2({-1, 0});

  // character1->velocity = Vector2(2, 0);
  // character2->velocity = Vector2(-2, 1);

  // Play music
  // music.Play("./assets/music/main.mp3");
}
