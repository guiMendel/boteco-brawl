#include "CharacterSelectScene.h"
#include "ObjectRecipes.h"
#include "Canvas.h"
#include "UIContainer.h"
#include "UIImage.h"

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
  mainContainer->Flexbox().placeItems = {0.5, 0.5};

  // Give it a background
  auto background = mainContainer->AddChild<UIImage>("Background", "./assets/images/character-selection/background.png");
  // background->height.Set(UIDimension::Percent, 100);
  background->SetSizePreserveRatio(UIDimension::Vertical, UIDimension::Percent, 100);
}
