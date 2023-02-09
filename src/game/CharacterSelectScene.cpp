#include "CharacterSelectScene.h"
#include "ObjectRecipes.h"
#include "Canvas.h"
#include "UIContainer.h"
#include "UIImage.h"

using namespace std;

void CharacterSelectScene::InitializeObjects()
{
  // Create the main camera
  Instantiate("MainCamera", ObjectRecipes::Camera());

  // Create main UI canvas
  auto canvas = Instantiate("Canvas", ObjectRecipes::Canvas(Canvas::Space::Global))->RequireComponent<Canvas>();

  // Add main container
  auto mainContainer = canvas->AddChild<UIContainer>("Main");
  mainContainer->width.Set(UIDimension::Percent, 100);
  mainContainer->height.Set(UIDimension::Percent, 100);
  mainContainer->padding.Set(UIDimension::Percent, 10);
  mainContainer->Flexbox().placeItems = {0.5, 0};
  mainContainer->Flexbox().mainAxis = UIDimension::Vertical;
  mainContainer->Flexbox().gap.Set(UIDimension::Percent, 5);

  // === BACKGROUND

  // Give it a background
  auto background = mainContainer->AddChild<UIImage>("Background", "./assets/images/character-selection/background.png");
  background->SetSizePreserveRatio(UIDimension::Vertical, UIDimension::Percent, 100);

  // Position background absolutely on the center
  background->SetPositionAbsolute(true);

  // Set other images' scaling to the background scaling
  mainContainer->style->imageScaling.Set(background->GetScaling());

  // === MAIN SECTIONS

  // Header with back button
  auto header = mainContainer->AddChild<UIContainer>("Header");

  // Character options
  auto characters = mainContainer->AddChild<UIContainer>("CharacterOptions");

  // Player selections
  auto selections = mainContainer->AddChild<UIContainer>("PlayerSelections");

  // === HEADER

  // Give it 100% of width
  header->width.Set(UIDimension::Percent, 92);
  header->Flexbox().gap.Set(UIDimension::RealPixels, 25);

  header->AddChild<UIImage>("BackButton", "./assets/images/character-selection/header/back-button.png");
  header->AddChild<UIImage>("Splash", "./assets/images/character-selection/header/header-splash.png");

  // === OPTIONS

  characters->AddChild<UIImage>("CharacterGrid", "./assets/images/character-selection/character-options/options.png");

  // === PLAYER SELECTIONS

  selections->Flexbox().gap.Set(UIDimension::Percent, 5);

  // Add a player section given it's assets
  auto addPlayerSection = [selections](string billPath, string connectPath)
  {
    auto playerSection = selections->AddChild<UIContainer>("PlayerSection");
    playerSection->Flexbox().placeItems = {0.5, 0.5};

    // Add connect prompt
    auto connectPrompt = playerSection->AddChild<UIImage>("ConnectPrompt", connectPath);
    connectPrompt->SetPositionAbsolute(true);
    connectPrompt->offset.y.Set(UIDimension::Percent, -10);

    // Add bill
    auto bill = playerSection->AddChild<UIImage>("Bill", billPath);
    bill->offset.y.Set(UIDimension::Percent, 65);

    return playerSection;
  };

  // Player 1
  addPlayerSection("./assets/images/character-selection/selections/bill-1.png",
                   "./assets/images/character-selection/selections/prompt-2.png");

  // Player 2
  addPlayerSection("./assets/images/character-selection/selections/bill-2.png",
                   "./assets/images/character-selection/selections/prompt-2.png");

  // Player 3
  addPlayerSection("./assets/images/character-selection/selections/bill-3.png",
                   "./assets/images/character-selection/selections/prompt-3.png");

  // Player 4
  addPlayerSection("./assets/images/character-selection/selections/bill-4.png",
                   "./assets/images/character-selection/selections/prompt-4.png");
}
