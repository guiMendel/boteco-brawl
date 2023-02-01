#include "CharacterUIManager.h"
#include "CharacterController.h"
#include "Camera.h"
#include "Heat.h"
#include <sstream>
#include <iomanip>

using namespace std;

const float CharacterUIManager::textSizeMultiplierDecay{3};

CharacterUIManager::CharacterUIManager(
    GameObject &associatedObject, shared_ptr<FallDeath> fallDeath, shared_ptr<UIText> heatText, shared_ptr<UIImage> lifeIcon)
    : UIComponent(associatedObject),
      weakFallDeath(fallDeath),
      lifeIcon(lifeIcon),
      weakText(heatText),
      playerColor(fallDeath->worldObject.RequireComponent<CharacterController>()->GetPlayer()->GetColor())
{
  LOCK(weakText, text);
  // originalTextSize = text->GetFontSize();
}

void CharacterUIManager::Start()
{
  LOCK(weakFallDeath, fallDeath);

  // Update life counter on fall
  fallDeath->OnFall.AddListener("update-lives", [this]()
                                { UpdateLifeCounter(); });

  // Start lives counter
  UpdateLifeCounter();

  // Grab heat
  auto heat = fallDeath->worldObject.RequireComponent<Heat>();

  heat->OnHeatChange.AddListener("update-heat-display", [this](float newHeat, float oldHeat)
                                 { UpdateHeatDisplay(newHeat, oldHeat); });

  fallDeath->OnFall.AddListener("update-heat-display", [this]()
                                { UpdateHeatDisplay(0, 0); });

  // Start heat display
  UpdateHeatDisplay(heat->GetHeat(), heat->GetHeat());
}

void CharacterUIManager::Update(float deltaTime)
{
  if (textSizeModifier != 1)
  {
    if (textSizeModifier > 1)
      textSizeModifier = max(1.0f, textSizeModifier - textSizeMultiplierDecay * deltaTime);
    else
      textSizeModifier = min(1.0f, textSizeModifier + textSizeMultiplierDecay * deltaTime);

    // Apply new multiplier
    LOCK(weakText, text);

    // text->SetFontSize(round(originalTextSize * textSizeModifier));
  }
}

void CharacterUIManager::EraseLives() const
{
}

void CharacterUIManager::UpdateLifeCounter() const
{
  LOCK(weakFallDeath, fallDeath);

  // Erase current lives
  EraseLives();

  // If no more, stop
  if (fallDeath->GetLives() == 0)
    return;

  cout << "Current lives: " << fallDeath->GetLives() << endl;

  // How many units to space lives out with
  // const float livesGap{0.1};

  // Offset of first life
  // float lifeSizeUnits = lifeSizeRealPixels / Camera::GetMain()->GetRealPixelsPerUnit();
  // const float firstOffset{(livesGap + lifeSizeUnits) * (fallDeath->GetLives() - 1) / 2};

  // // For each life
  // for (int life{0}; life < fallDeath->GetLives(); life++)
  // {
  //   auto lifeRenderer = worldObject.AddComponent<SpriteRenderer>(spritePath, RenderLayer::UI);
  //   lifeRenderer->OverrideWidthPixels(lifeSizeRealPixels);
  //   lifeRenderer->SetAnchorPoint({0.5, 1});
  //   lifeRenderer->SetOffset({-firstOffset + life * (livesGap + lifeSizeUnits), 0});
  //   lifeRenderer->SetColor(color);
  // }
}

void CharacterUIManager::UpdateHeatDisplay(float newHeat, float oldHeat)
{
  LOCK(weakText, text);

  // Convert heat to 1 decimal number string
  stringstream damageString;
  damageString << fixed << setprecision(1) << newHeat;

  // Set the text
  text->SetText(damageString.str());

  // Give it an appropriate render order
  // text->renderOrder = SDL_GetTicks();

  // Set a size pop effect
  textSizeModifier = min(textSizeModifier + log10f(abs(newHeat - oldHeat) + 5), 5.0f);

  // Give it a color proportional to heat
  // float heatProportion = newHeat / 100.0f;
  // text->SetColor(Lerp(Color::White(), Color::Red(), heatProportion));
  // text->SetBorderColor(Lerp(Color::Black(), Color(70, 0, 0), heatProportion));
}

void CharacterUIManager::ShowHeatDisplay(bool show)
{
  LOCK(weakText, text);

  text->SetEnabled(show);
}
