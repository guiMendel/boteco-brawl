#include "MainScene.h"
#include "GameData.h"
#include "ObjectRecipes.h"
#include "PlayerManager.h"
#include "ParticleFX.h"
#include "Camera.h"
#include "TimeScaleManager.h"
#include "ShakeEffectManager.h"

using namespace std;

void MainScene::LoadAssets()
{
}

void MainScene::OnUpdate(float)
{
  // auto thing = FindObject("Thing");
  // cout << "Thing at: " << thing->GetPosition() << endl;
}

void MainScene::InitializeObjects()
{
  // Object to hold character
  auto charactersParent = CreateObject("CharactersParent");

  // Create the main camera
  auto mainCamera = CreateObject("MainCamera", ObjectRecipes::Camera(charactersParent))->GetComponent<Camera>();

  // Add player manager
  auto playerManager = CreateObject("PlayerManager", ObjectRecipes::SingleComponent<PlayerManager>(true))->RequireComponent<PlayerManager>();

  // Add player 2
  auto player2 = playerManager->AddNewPlayer();

  // Add particle fx
  CreateObject("ParticleFX", ObjectRecipes::SingleComponent<ParticleFX>());

  // Add time scale manager
  CreateObject("TimeScaleManager", ObjectRecipes::SingleComponent<TimeScaleManager>());

  // Add a shake effect manager
  CreateObject("ShakeEffectManager", ObjectRecipes::SingleComponent<ShakeEffectManager>());

  // Add an arena
  CreateObject("Arena", ObjectRecipes::Arena("./assets/images/boteco.jpg"));

  // Add player
  auto character1 = CreateObject(
                        "Character-1",
                        ObjectRecipes::Character(playerManager->GetMainPlayer()),
                        Vector2{-3.5, -10},
                        0,
                        charactersParent)
                        ->GetComponent<Rigidbody>();

  auto character2 = CreateObject(
                        "Character-2",
                        ObjectRecipes::Character(player2),
                        Vector2{3.5, -10},
                        0,
                        charactersParent)
                        ->GetComponent<Rigidbody>();

  CreateObject("Ground", ObjectRecipes::Platform({40, 2}), Vector2(0, 4));
  CreateObject("LeftWall", ObjectRecipes::Platform({2, 10}), Vector2(-20, 2));
  CreateObject("RightWall", ObjectRecipes::Platform({2, 10}), Vector2(20, 2));

  CreateObject("Platform", ObjectRecipes::Platform({2, 0.5}, true), Vector2(0, 0));

  // FindObjectOfType<ShakeEffectManager>()->Shake(
  //     character1->worldObject.GetShared(), DegreesToRadians(-45), {1, 0.1}, {1, 0.05}, 8);

  // character1->velocity = Vector2(2, 0);
  // character2->velocity = Vector2(-2, 1);

  // Play music
  // music.Play("./assets/music/main.mp3");
}
