#include "CharacterLifeDisplay.h"
#include "CharacterController.h"
#include "Camera.h"

using namespace std;

CharacterLifeDisplay::CharacterLifeDisplay(
    GameObject &associatedObject, shared_ptr<FallDeath> fallDeath, float lifeSize, std::string spritePath)
    : WorldComponent(associatedObject), weakFallDeath(fallDeath), lifeSizeRealPixels(lifeSize), spritePath(spritePath)
{
  color = fallDeath->worldObject.RequireComponent<CharacterController>()->GetPlayer()->GetColor();
}

void CharacterLifeDisplay::Start()
{
  LOCK(weakFallDeath, fallDeath);

  fallDeath->OnFall.AddListener("update-lives", [this]()
                                { UpdateDisplay(); });

  UpdateDisplay();
}

void CharacterLifeDisplay::EraseLives() const
{
  auto renderers = worldObject.GetComponents<SpriteRenderer>();

  for (auto renderer : renderers)
    worldObject.RemoveComponent(renderer);
}

void CharacterLifeDisplay::UpdateDisplay() const
{
  LOCK(weakFallDeath, fallDeath);

  // Erase current lives
  EraseLives();

  // If no more, stop
  if (fallDeath->GetLives() == 0)
    return;

  // How many units to space lives out with
  const float livesGap{0.1};

  // Offset of first life
  float lifeSizeUnits = lifeSizeRealPixels / Camera::GetMain()->GetRealPixelsPerUnit();
  const float firstOffset{(livesGap + lifeSizeUnits) * (fallDeath->GetLives() - 1) / 2};

  // For each life
  for (int life{0}; life < fallDeath->GetLives(); life++)
  {
    auto lifeRenderer = worldObject.AddComponent<SpriteRenderer>(spritePath, RenderLayer::UI);
    lifeRenderer->OverrideWidthPixels(lifeSizeRealPixels);
    lifeRenderer->SetAnchorPoint({0.5, 1});
    lifeRenderer->SetOffset({-firstOffset + life * (livesGap + lifeSizeUnits), 0});
    lifeRenderer->SetColor(color);
  }
}
