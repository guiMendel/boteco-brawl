#include "MainScene.h"
#include "GameData.h"
#include "ObjectRecipes.h"
#include "PlayerManager.h"
#include "ParticleFX.h"
#include "Camera.h"
#include "TimeScaleManager.h"
#include "ShakeEffectManager.h"
#include "CameraBehavior.h"
#include "CharacterTest.h"
#include "CharacterKafta.h"
#include "CharacterKiba.h"
#include "BrawlPlayer.h"

using namespace std;

// Initial space between characters
static const float initialCharactersDistance{3};

void MainScene::OnUpdate(float)
{
}

void MainScene::InitializeObjects()
{
  // Object to hold characters
  auto charactersParent = Instantiate(CHARACTERS_PARENT);

  // Create the main camera
  auto mainCamera = Instantiate("MainCamera", ObjectRecipes::Camera())->GetComponent<Camera>();

  // Add particle fx
  Instantiate("ParticleFX", ObjectRecipes::SingleComponent<ParticleFX>());

  // Add time scale manager
  Instantiate("TimeScaleManager", ObjectRecipes::SingleComponent<TimeScaleManager>());

  // Add a shake effect manager
  Instantiate("ShakeEffectManager", ObjectRecipes::SingleComponent<ShakeEffectManager>());

  // Give it behavior
  mainCamera->gameObject.AddComponent<CameraBehavior>(charactersParent);

  SetupArena();

  // Get player manager
  auto playerManager = FindComponent<PlayerManager>();

  // If this was the starting scene, add some sample players
  if (playerManager == nullptr)
  {
    // Add player manager
    playerManager = Instantiate("PlayerManager", ObjectRecipes::SingleComponent<PlayerManager>(true))->RequireComponent<PlayerManager>();

    playerManager->AddNewPlayer<BrawlPlayer>()->SetCharacter<CharacterKafta>();
    playerManager->AddNewPlayer<BrawlPlayer>()->SetCharacter<CharacterKiba>();
  }

  SpawnCharacters();
}

void MainScene::SpawnCharacters()
{
  auto playerManager = RequireFindComponent<PlayerManager>();
  auto charactersParent = RequireWorldObject(CHARACTERS_PARENT);

  // Get how many characters there will be
  int characterCount = playerManager->GetPlayers().size();

  // Initialize character offset
  float offset = initialCharactersDistance * (characterCount - 1) / 2;

  // For each player
  for (auto player : playerManager->GetPlayers())
  {
    Instantiate(
        "Character" + to_string(player->PlayerId()),
        RequirePointerCast<BrawlPlayer>(player)->characterRecipe,
        Vector2{offset, -10},
        0,
        charactersParent);
  }
}

void MainScene::SetupArena()
{
  // Add an arena
  Instantiate("Arena", ObjectRecipes::Arena("./assets/images/boteco.jpg"));

  Instantiate("Ground", ObjectRecipes::Platform({40, 2}), Vector2(0, 4));
  Instantiate("LeftWall", ObjectRecipes::Platform({2, 10}), Vector2(-20, 2));
  Instantiate("RightWall", ObjectRecipes::Platform({2, 10}), Vector2(20, 2));

  Instantiate("Platform", ObjectRecipes::Platform({2, 0.5}, true), Vector2(0, 0));
}
