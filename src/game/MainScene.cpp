#include "MainScene.h"
#include "GameData.h"
#include "ObjectRecipes.h"
#include "PlayerManager.h"
#include "ParticleFX.h"
#include "Camera.h"
#include "TimeScaleManager.h"
#include "ShakeEffectManager.h"

using namespace std;

void MainScene::OnUpdate(float)
{
}

void MainScene::InitializeObjects()
{
  // Object to hold character
  auto charactersParent = CreateWorldObject("CharactersParent");

  // Create the main camera
  auto mainCamera = CreateWorldObject("MainCamera", ObjectRecipes::Camera(charactersParent))->GetComponent<Camera>();

  // Add player manager
  auto playerManager = CreateWorldObject("PlayerManager", ObjectRecipes::SingleComponent<PlayerManager>(true))->RequireComponent<PlayerManager>();

  // Add player 2
  auto player2 = playerManager->AddNewPlayer();

  // Add particle fx
  CreateWorldObject("ParticleFX", ObjectRecipes::SingleComponent<ParticleFX>());

  // Add time scale manager
  CreateWorldObject("TimeScaleManager", ObjectRecipes::SingleComponent<TimeScaleManager>());

  // Add a shake effect manager
  CreateWorldObject("ShakeEffectManager", ObjectRecipes::SingleComponent<ShakeEffectManager>());

  // Add an arena
  CreateWorldObject("Arena", ObjectRecipes::Arena("./assets/images/boteco.jpg"));

  // Add player
  auto character1 = CreateWorldObject(
                        "Character-1",
                        ObjectRecipes::Character(playerManager->GetMainPlayer()),
                        Vector2{-3.5, -10},
                        0,
                        charactersParent)
                        ->GetComponent<Rigidbody>();

  auto character2 = CreateWorldObject(
                        "Character-2",
                        ObjectRecipes::Character(player2),
                        Vector2{3.5, -10},
                        0,
                        charactersParent)
                        ->GetComponent<Rigidbody>();

  CreateWorldObject("Ground", ObjectRecipes::Platform({40, 2}), Vector2(0, 4));
  CreateWorldObject("LeftWall", ObjectRecipes::Platform({2, 10}), Vector2(-20, 2));
  CreateWorldObject("RightWall", ObjectRecipes::Platform({2, 10}), Vector2(20, 2));

  CreateWorldObject("Platform", ObjectRecipes::Platform({2, 0.5}, true), Vector2(0, 0));

  // FindComponent<ShakeEffectManager>()->Shake(
  //     character1->worldObject.GetShared(), DegreesToRadians(-45), {1, 0.1}, {1, 0.05}, 8);

  // character1->velocity = Vector2(2, 0);
  // character2->velocity = Vector2(-2, 1);

  // Play music
  // music.Play("./assets/music/main.mp3");
}
