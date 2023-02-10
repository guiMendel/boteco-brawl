#include "SplashScene.h"
#include "ObjectRecipes.h"
#include "Canvas.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UIBackground.h"
#include "SplashAnimation.h"
#include "ParticleEmitter.h"
#include "MainMenuInput.h"

using namespace std;

void SplashScene::InitializeObjects()
{
  // Create the main camera
  Instantiate("MainCamera", ObjectRecipes::Camera());

  // Create main UI canvas
  auto canvas = Instantiate("Canvas", ObjectRecipes::Canvas(Canvas::Space::Global))->RequireComponent<Canvas>();

  // Add input
  auto input = Instantiate("InputHandler", ObjectRecipes::SingleComponent<MainMenuInput>());

  // Add main container
  auto mainContainer = canvas->AddChild<UIContainer>("Main");
  mainContainer->width.Set(UIDimension::Percent, 100);
  mainContainer->height.Set(UIDimension::Percent, 100);
  mainContainer->Flexbox().placeItems = {0.5, 0.5};
  mainContainer->Flexbox().mainAxis = UIDimension::Vertical;
  mainContainer->Flexbox().gap.Set(UIDimension::Percent, 10);

  // === BACKGROUND

  // Give it a background
  auto background = mainContainer->AddChild<UIImage>("Background", "./assets/images/splash-screen/background.png");
  background->SetSizePreserveRatio(UIDimension::Vertical, UIDimension::Percent, 100);

  // Position background absolutely on the center
  background->SetPositionAbsolute(true);

  // Set other images' scaling to the background scaling
  mainContainer->style->imageScaling.Set(background->GetScaling());

  // === CONTENT

  // Splash art
  auto splash = mainContainer->AddChild<UIImage>("Splash", "./assets/images/splash-screen/splash.png");
  splash->localScale = {40, 40};
  splash->style->renderOrder.Set(2);
  splash->margin.bottom.Set(UIDimension::Percent, -10);

  // Splash subtitle
  auto subtitle = mainContainer->AddChild<UIImage>("Subtitle", "./assets/images/splash-screen/subtitle.png");
  subtitle->SetEnabled(false);
  subtitle->style->renderOrder.Set(5);

  // Press start
  auto prompt = mainContainer->AddChild<UIImage>("StartPrompt", "./assets/images/splash-screen/press-start.png");
  prompt->style->imageColor.Set(Color(255, 255, 255, 0));

  // Funny text
  mainContainer->AddChild<UIImage>("Text", "./assets/images/splash-screen/text.png");

  // === CURTAIN

  auto curtain = mainContainer->AddChild<UIContainer>("Curtain")->AddComponent<UIBackground>(Color::Black());
  curtain->uiObject.style->renderOrder.Set(10);
  curtain->uiObject.SetPositionAbsolute(true);
  curtain->uiObject.width.Set(UIDimension::Percent, 100);
  curtain->uiObject.height.Set(UIDimension::Percent, 100);

  // === ANIMATION

  // Add particle emitter for splash stomp
  auto stompParticles = NewObject<WorldObject>("StompParticles")->AddComponent<ParticleEmitter>(RenderLayer::UI, make_unique<Rectangle>(splash->width.As(UIDimension::WorldUnits), splash->height.As(UIDimension::WorldUnits)), false, 0.01);
  stompParticles->emission.color = {Color(225, 170, 116), Color(185, 130, 76)};
  stompParticles->emission.frequency = {0.0001, 0.00001};
  stompParticles->emission.lifetime = {0.5, 5};
  stompParticles->emission.speed = {1, 2};
  stompParticles->emission.gravityModifier = {Vector2::Down(0.05), Vector2::Down(0.1)};
  stompParticles->renderOrder = 15;
  stompParticles->emitOnStart = false;

  // Add animation handler
  mainContainer->AddComponent<SplashAnimation>(splash, subtitle, prompt, curtain, stompParticles);
}
