#include "MainScene.h"
#include "UIBackground.h"
#include "UIImage.h"
#include "ArenaUIAnimation.h"
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

  // Add canvas
  auto canvas = Instantiate("Canvas", ObjectRecipes::Canvas(Canvas::Space::Global))->RequireComponent<Canvas>();

  auto countdownContainer = canvas->AddChild<UIContainer>("CountdownContainer");
  countdownContainer->width.Set(UIDimension::Percent, 100);
  countdownContainer->height.Set(UIDimension::Percent, 100);
  countdownContainer->Flexbox().placeItems = {0, 0.5};
  countdownContainer->padding.left.Set(UIDimension::Percent, 100);
  countdownContainer->style->textBorderSize.Set(4);
  countdownContainer->style->textBorderColor.Set(PLAYER_1_COLOR);
  countdownContainer->style->fontSize.Set(110);
  countdownContainer->style->textColor.Set(PLAYER_2_COLOR);

  // Add curtain
  auto curtain = countdownContainer->AddChild<UIContainer>(CURTAIN_OBJECT);
  curtain->AddComponent<UIBackground>(Color::Black());
  curtain->SetPositionAbsolute(true);
  curtain->width.Set(UIDimension::Percent, 100);
  curtain->height.Set(UIDimension::Percent, 100);
  curtain->style->renderOrder.Set(10);

  // Add each countdown object
  for (auto text : {"3", "2", "1", "Rinha!"})
  {
    auto textContainer = countdownContainer->AddChild<UIContainer>("CountdownContainer" + string(text));
    textContainer->width.Set(UIDimension::Percent, 100);
    textContainer->height.Set(UIDimension::Percent, 100);
    textContainer->Flexbox().placeItems = {0.5, 0.5};

    auto textObject = textContainer->AddChild<UIText>("Countdown" + string(text), text);
  }

  // Add animation
  countdownContainer->AddComponent<ArenaUIAnimation>();
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

    offset -= initialCharactersDistance;
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
