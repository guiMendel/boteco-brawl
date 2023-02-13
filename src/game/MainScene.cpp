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
  // Will hold all in-game objects
  auto mainParent = Instantiate(MAIN_PARENT_OBJECT);

  // Add time scale manager
  Instantiate("TimeScaleManager", ObjectRecipes::SingleComponent<TimeScaleManager>());

  // Object to hold characters
  auto charactersParent = mainParent->CreateChild(CHARACTERS_PARENT);

  // Create the main camera
  auto mainCamera = Instantiate("MainCamera", ObjectRecipes::Camera())->GetComponent<Camera>();
  mainCamera->worldObject.SetParent(mainParent);

  // Give it behavior
  mainCamera->gameObject.AddComponent<CameraBehavior>(charactersParent);

  // Add particle fx
  Instantiate("ParticleFX", ObjectRecipes::SingleComponent<ParticleFX>())->SetParent(mainParent);

  // Add a shake effect manager
  Instantiate("ShakeEffectManager", ObjectRecipes::SingleComponent<ShakeEffectManager>())->SetParent(mainParent);

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

  playerManager->worldObject.SetParent(mainParent);

  SpawnCharacters();

  // Add canvas
  auto canvas = Instantiate("Canvas", ObjectRecipes::Canvas(Canvas::Space::Global))->RequireComponent<Canvas>();

  // Add curtain
  auto curtain = canvas->AddChild<UIContainer>(CURTAIN_OBJECT);
  curtain->AddComponent<UIBackground>(Color::Black());
  curtain->SetPositionAbsolute(true);
  curtain->width.Set(UIDimension::Percent, 100);
  curtain->height.Set(UIDimension::Percent, 100);
  curtain->style->renderOrder.Set(10);

  // Add countdown container
  auto countdownContainer = canvas->AddChild<UIContainer>(COUNTDOWN_OBJECT);
  countdownContainer->width.Set(UIDimension::Percent, 100);
  countdownContainer->height.Set(UIDimension::Percent, 100);
  countdownContainer->Flexbox().placeItems = {0, 0.5};
  countdownContainer->padding.left.Set(UIDimension::Percent, 100);
  countdownContainer->style->textBorderSize.Set(4);
  countdownContainer->style->textBorderColor.Set(PLAYER_1_COLOR);
  countdownContainer->style->fontSize.Set(110);
  countdownContainer->style->textColor.Set(PLAYER_2_COLOR);

  // Add each countdown object
  for (auto text : {"3", "2", "1", "Rinha!"})
  {
    auto textContainer = countdownContainer->AddChild<UIContainer>("CountdownContainer" + string(text));
    textContainer->width.Set(UIDimension::Percent, 100);
    textContainer->height.Set(UIDimension::Percent, 100);
    textContainer->Flexbox().placeItems = {0.5, 0.5};

    auto textObject = textContainer->AddChild<UIText>("Countdown" + string(text), text);
  }

  // Add victory text
  auto victoryContainer = canvas->AddChild<UIContainer>("VictoryContainer");
  victoryContainer->width.Set(UIDimension::Percent, 100);
  victoryContainer->height.Set(UIDimension::Percent, 100);
  victoryContainer->Flexbox().placeItems = {0.5, 0.5};
  victoryContainer->margin.left.Set(UIDimension::Percent, 120);
  victoryContainer->style->textBorderSize.Set(4);
  victoryContainer->style->textBorderColor.Set(PLAYER_1_COLOR);
  victoryContainer->style->fontSize.Set(110);
  victoryContainer->style->textColor.Set(PLAYER_2_COLOR);

  victoryContainer->AddChild<UIText>(VICTORY_TEXT, " ");

  // Add animation
  canvas->worldObject.AddComponent<ArenaUIAnimation>();
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
  auto mainParent = RequireWorldObject(MAIN_PARENT_OBJECT);

  // Add an arena
  Instantiate("Arena", ObjectRecipes::Arena("./assets/images/boteco.jpg"))->SetParent(mainParent);

  Instantiate("Ground", ObjectRecipes::Platform({40, 2}), Vector2(0, 4))->SetParent(mainParent);
  Instantiate("LeftWall", ObjectRecipes::Platform({2, 10}), Vector2(-20, 2))->SetParent(mainParent);
  Instantiate("RightWall", ObjectRecipes::Platform({2, 10}), Vector2(20, 2))->SetParent(mainParent);

  Instantiate("Platform", ObjectRecipes::Platform({2, 0.5}, true), Vector2(0, 0))->SetParent(mainParent);
}
