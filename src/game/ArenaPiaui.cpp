#include "ArenaPiaui.h"
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

  // Add background
  worldObject.AddComponent<SpriteRenderer>(
      "./assets/sprites/piaui/background.png",
      RenderLayer::Background);

  GetScene()->Instantiate("Platform", ObjectRecipes::Platform({30, 3}), Vector2{0, 5})->SetParent(worldObject.GetShared());
}
