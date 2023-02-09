#include "SplashScene.h"
#include "ObjectRecipes.h"
#include "Canvas.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UIBackground.h"
#include "SplashAnimation.h"

using namespace std;

void SplashScene::InitializeObjects()
{
  // Create the main camera
  Instantiate("MainCamera", ObjectRecipes::Camera());

  // Create main UI canvas
  auto canvas = Instantiate("Canvas", ObjectRecipes::Canvas(Canvas::Space::Global))->RequireComponent<Canvas>();

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
  splash->localScale = {20, 20};

  // Press start
  mainContainer->AddChild<UIImage>("Splash", "./assets/images/splash-screen/press-start.png");

  // Funny text
  mainContainer->AddChild<UIImage>("Splash", "./assets/images/splash-screen/text.png");

  // === CURTAIN

  auto curtain = mainContainer->AddChild<UIContainer>("Curtain")->AddComponent<UIBackground>(Color::Black());
  curtain->uiObject.style->renderOrder.Set(10);
  curtain->uiObject.SetPositionAbsolute(true);
  curtain->uiObject.width.Set(UIDimension::Percent, 100);
  curtain->uiObject.height.Set(UIDimension::Percent, 100);

  // === ANIMATION

  mainContainer->AddComponent<SplashAnimation>(splash, nullptr, curtain);
}
