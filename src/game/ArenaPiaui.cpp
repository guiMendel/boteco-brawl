#include "ArenaPiaui.h"
#include "CameraBehavior.h"
#include "ObjectRecipes.h"
#include "SpriteRenderer.h"

using namespace std;

ArenaPiaui::ArenaPiaui(GameObject &associatedObject) : Arena(associatedObject) {}

void ArenaPiaui::InitializeArena()
{
  // Add platforms sprite
  auto platforms = worldObject.AddComponent<SpriteRenderer>(
      "./assets/sprites/piaui/platforms.png",
      RenderLayer::Platforms);

  // Set dimensions according to this size
  width = platforms->GetSprite()->GetWidth();
  height = platforms->GetSprite()->GetHeight();

  // Get max camera size
  float maxCameraSize = GetScene()->RequireFindComponent<CameraBehavior>()->GetMaxCameraSize();

  // Add background
  auto background = worldObject.AddComponent<SpriteRenderer>(
      "./assets/sprites/piaui/background.png",
      RenderLayer::Background);

  // Give background a parallax
  background->SetParallax(0.5, maxCameraSize);

  GetScene()->Instantiate("Platform", ObjectRecipes::Platform({30, 3}), Vector2{0, 5})->SetParent(worldObject.GetShared());
}
