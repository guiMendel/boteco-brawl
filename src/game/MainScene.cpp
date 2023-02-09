#include "MainScene.h"
#include "GameData.h"
#include "ObjectRecipes.h"
#include "PlayerManager.h"
#include "ParticleFX.h"
#include "Camera.h"
#include "TimeScaleManager.h"
#include "ShakeEffectManager.h"
#include "CameraBehavior.h"

using namespace std;

void MainScene::OnUpdate(float)
{
}

void MainScene::InitializeObjects()
{
  // Object to hold character
  auto charactersParent = Instantiate("CharactersParent");

  // Create the main camera
  auto mainCamera = Instantiate("MainCamera", ObjectRecipes::Camera())->GetComponent<Camera>();

  // Give it behavior
  mainCamera->gameObject.AddComponent<CameraBehavior>(charactersParent);

  // Add player manager
  auto playerManager = Instantiate("PlayerManager", ObjectRecipes::SingleComponent<PlayerManager>(true))->RequireComponent<PlayerManager>();

  // Add player 2
  auto player2 = playerManager->AddNewPlayer();

  // Add particle fx
  Instantiate("ParticleFX", ObjectRecipes::SingleComponent<ParticleFX>());

  // Add time scale manager
  Instantiate("TimeScaleManager", ObjectRecipes::SingleComponent<TimeScaleManager>());

  // Add a shake effect manager
  Instantiate("ShakeEffectManager", ObjectRecipes::SingleComponent<ShakeEffectManager>());

  // Add an arena
  Instantiate("Arena", ObjectRecipes::Arena("./assets/images/boteco.jpg"));

  // Add player
  auto character1 = Instantiate(
                        "Character-1",
                        ObjectRecipes::Character(playerManager->GetMainPlayer()),
                        Vector2{-3.5, -10},
                        0,
                        charactersParent)
                        ->GetComponent<Rigidbody>();

  auto character2 = Instantiate(
                        "Character-2",
                        ObjectRecipes::Character(player2),
                        Vector2{3.5, -10},
                        0,
                        charactersParent)
                        ->GetComponent<Rigidbody>();

  Instantiate("Ground", ObjectRecipes::Platform({40, 2}), Vector2(0, 4));
  Instantiate("LeftWall", ObjectRecipes::Platform({2, 10}), Vector2(-20, 2));
  Instantiate("RightWall", ObjectRecipes::Platform({2, 10}), Vector2(20, 2));

  Instantiate("Platform", ObjectRecipes::Platform({2, 0.5}, true), Vector2(0, 0));

  // FindComponent<ShakeEffectManager>()->Shake(
  //     character1->worldObject.GetShared(), DegreesToRadians(-45), {1, 0.1}, {1, 0.05}, 8);

  // character1->velocity = Vector2(2, 0);
  // character2->velocity = Vector2(-2, 1);

  // Play music
  // music.Play("./assets/music/main.mp3");
}
