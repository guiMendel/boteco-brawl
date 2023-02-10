#include "MenuScene.h"
#include "ObjectRecipes.h"
#include "Canvas.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UIBackground.h"
#include "SplashAnimation.h"
#include "ParticleEmitter.h"
#include "MainMenuInput.h"

using namespace std;

#define CURTAIN_OBJECT "Curtain"
#define SPLASH_OBJECT "Splash"
#define SUBTITLE_OBJECT "Subtitle"
#define PROMPT_OBJECT "StartPrompt"
#define PARTICLES_OBJECT "StompParticles"

void MenuScene::InitializeObjects()
{
  // Create the main camera
  auto camera = Instantiate("MainCamera", ObjectRecipes::Camera())->RequireComponent<Camera>();
  camera->background = Color(226, 160, 106);

  // Create main UI canvas
  auto canvas = Instantiate("Canvas", ObjectRecipes::Canvas(Canvas::Space::Global))->RequireComponent<Canvas>();

  auto mainContainer = canvas->AddChild<UIContainer>("Main");
  mainContainer->width.Set(UIDimension::Percent, 100);
  mainContainer->height.Set(UIDimension::Percent, 100);
  mainContainer->Flexbox().placeItems = {0.5, 0};
  mainContainer->Flexbox().mainAxis = UIDimension::Vertical;
  // mainContainer->Flexbox().gap.Set(UIDimension::Percent, 5);
  mainContainer->padding.top.Set(UIDimension::Percent, 0);

  // === BACKGROUND

  // Give it a background
  auto background = mainContainer->AddChild<UIImage>("Background", "./assets/images/character-selection/background.png");
  background->SetSizePreserveRatio(UIDimension::Vertical, UIDimension::Percent, 100);

  // Position background absolutely on the center
  background->SetPositionAbsolute(true);

  // Set other images' scaling to the background scaling
  mainContainer->style->imageScaling.Set(background->GetScaling());

  // === CURTAIN

  auto curtain = mainContainer->AddChild<UIContainer>(CURTAIN_OBJECT)->AddComponent<UIBackground>(Color::Black());
  curtain->uiObject.style->renderOrder.Set(10);
  curtain->uiObject.SetPositionAbsolute(true);
  curtain->uiObject.width.Set(UIDimension::Percent, 100);
  curtain->uiObject.height.Set(UIDimension::Percent, 100);

  CreateSplash(mainContainer);
  CreateSelection(mainContainer);

  // Add animation handler
  auto animationHandler = mainContainer->AddComponent<SplashAnimation>(
      mainContainer,
      RequireUIObject<UIImage>(SPLASH_OBJECT),
      RequireUIObject<UIImage>(SUBTITLE_OBJECT),
      RequireUIObject<UIImage>(PROMPT_OBJECT),
      curtain,
      RequireWorldObject(PARTICLES_OBJECT)->RequireComponent<ParticleEmitter>());

  // Add input
  NewObject<WorldObject>("InputHandler")->AddComponent<MainMenuInput>(animationHandler);
}

void MenuScene::CreateSplash(shared_ptr<UIContainer> mainContainer)
{
  // Add main container
  auto splashContainer = mainContainer->AddChild<UIContainer>("SplashContainer");
  splashContainer->width.Set(UIDimension::Percent, 100);
  splashContainer->height.Set(UIDimension::Percent, 100);
  splashContainer->Flexbox().placeItems = {0.5, 0.5};
  splashContainer->Flexbox().mainAxis = UIDimension::Vertical;
  splashContainer->Flexbox().gap.Set(UIDimension::Percent, 10);

  // === CONTENT

  // Splash art
  auto splash = splashContainer->AddChild<UIImage>(SPLASH_OBJECT, "./assets/images/splash-screen/splash.png");
  splash->localScale = {40, 40};
  splash->style->renderOrder.Set(2);
  splash->margin.bottom.Set(UIDimension::Percent, -10);

  // Splash subtitle
  auto subtitle = splashContainer->AddChild<UIImage>(SUBTITLE_OBJECT, "./assets/images/splash-screen/subtitle.png");
  subtitle->SetEnabled(false);
  subtitle->style->renderOrder.Set(5);

  // Press start
  auto prompt = splashContainer->AddChild<UIImage>(PROMPT_OBJECT, "./assets/images/splash-screen/press-start.png");
  prompt->style->imageColor.Set(Color(255, 255, 255, 0));

  // Funny text
  splashContainer->AddChild<UIImage>("Text", "./assets/images/splash-screen/text.png");

  // === ANIMATION

  // Add particle emitter for splash stomp
  auto stompParticles = NewObject<WorldObject>(PARTICLES_OBJECT)->AddComponent<ParticleEmitter>(RenderLayer::UI, make_unique<Rectangle>(splash->width.As(UIDimension::WorldUnits), splash->height.As(UIDimension::WorldUnits)), false, 0.01);
  stompParticles->emission.color = {Color(225, 170, 116), Color(185, 130, 76)};
  stompParticles->emission.frequency = {0.0001, 0.00001};
  stompParticles->emission.lifetime = {0.5, 5};
  stompParticles->emission.speed = {1, 2};
  stompParticles->emission.gravityModifier = {Vector2::Down(0.05), Vector2::Down(0.1)};
  stompParticles->renderOrder = 15;
  stompParticles->emitOnStart = false;
}

void MenuScene::CreateSelection(shared_ptr<UIContainer> mainContainer)
{
  // Add main container
  auto selectionContainer = mainContainer->AddChild<UIContainer>("SelectionContainer");
  selectionContainer->width.Set(UIDimension::Percent, 100);
  selectionContainer->height.Set(UIDimension::Percent, 100);
  selectionContainer->padding.Set(UIDimension::Percent, 10);
  selectionContainer->Flexbox().placeItems = {0.5, 0};
  selectionContainer->Flexbox().mainAxis = UIDimension::Vertical;
  selectionContainer->Flexbox().gap.Set(UIDimension::Percent, 5);

  // === MAIN SECTIONS

  // Header with back button
  auto header = selectionContainer->AddChild<UIContainer>("Header");

  // Character options
  auto optionsGrid = selectionContainer->AddChild<UIContainer>("CharacterOptions");

  // Player selections
  auto selections = selectionContainer->AddChild<UIContainer>("PlayerSelections");

  // === HEADER

  // Give it 100% of width
  header->width.Set(UIDimension::Percent, 92);
  header->Flexbox().gap.Set(UIDimension::RealPixels, 25);

  header->AddChild<UIImage>("BackButton", "./assets/images/character-selection/header/back-button.png");
  header->AddChild<UIImage>("Splash", "./assets/images/character-selection/header/header-splash.png");

  // === OPTIONS

  optionsGrid->Flexbox().wrap = true;
  optionsGrid->Flexbox().gap.x.Set(UIDimension::RealPixels, 55);
  optionsGrid->Flexbox().gap.y.Set(UIDimension::RealPixels, 40);
  optionsGrid->width.SetMax(UIDimension::Percent, 100);

  // Adds a character option given its assets
  auto addOption = [optionsGrid](string optionName, string optionDescriptionPath)
  {
    auto option = optionsGrid->AddChild<UIContainer>("Option" + optionName);
    option->Flexbox().placeItems = {0.5, 0.5};
    option->Flexbox().gap.Set(UIDimension::RealPixels, 10);

    option->AddChild<UIImage>("Portrait", "./assets/images/character-selection/character-options/option-portrait.png");
    option->AddChild<UIImage>("Description", optionDescriptionPath);
  };

  // Kafta
  addOption("Kafta", "./assets/images/character-selection/character-options/option-kafta.png");

  // Pastel
  addOption("Pastel", "./assets/images/character-selection/character-options/option-pastel.png");

  // Kiba
  addOption("Kiba", "./assets/images/character-selection/character-options/option-kiba.png");

  // Random
  addOption("Random", "./assets/images/character-selection/character-options/option-unknown.png");

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
