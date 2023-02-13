#include "CharacterUIManager.h"
#include "CharacterController.h"
#include "Camera.h"
#include "ObjectRecipes.h"
#include "Heat.h"
#include <sstream>
#include <iomanip>

using namespace std;

const float CharacterUIManager::textSizeMultiplierDecay{3};

CharacterUIManager::CharacterUIManager(
    GameObject &associatedObject, shared_ptr<FallDeath> fallDeath, shared_ptr<UIText> heatText, function<void()> addLifeIcon)
    : UIComponent(associatedObject),
      weakFallDeath(fallDeath),
      addLifeIcon(addLifeIcon),
      weakText(heatText),
      playerColor(fallDeath->worldObject.RequireComponent<CharacterController>()->GetPlayer()->GetColor()),
      uiContainer(*GetScene()->RequireUIObject<UIContainer>(uiObject.id))
{
  LOCK(weakText, text);
  originalTextSize = text->style->fontSize.Get();
}

void CharacterUIManager::Start()
{
  LOCK(weakFallDeath, fallDeath);

  // Get life container
  weakLifeContainer = RequirePointerCast<UIContainer>(uiContainer.RequireChild(CHARACTER_LIFE_OBJECT));

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

    text->style->fontSize.Set(originalTextSize * textSizeModifier);
  }
}

void CharacterUIManager::EraseLives() const
{
  for (auto lifeIcon : Lock(weakLifeContainer)->GetChildren())
    lifeIcon->RequestDestroy();
}

void CharacterUIManager::UpdateLifeCounter() const
{
  LOCK(weakFallDeath, fallDeath);

  // Erase current lives
  EraseLives();

  // If no more, stop
  if (fallDeath->GetLives() == 0)
    return;

  // For each life, add an icon
  for (int life{0}; life < fallDeath->GetLives(); life++)
    addLifeIcon();
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
  text->style->renderOrder.Set(SDL_GetTicks());

  // Set a size pop effect
  textSizeModifier = min(textSizeModifier + log10f(abs(newHeat - oldHeat) + 5), 5.0f);

  // Give it a color proportional to heat
  float heatProportion = newHeat / 100.0f;
  text->style->textColor.Set(Lerp(Color::White(), Color::Red(), heatProportion));
  text->style->textBorderColor.Set(Lerp(Color::Black(), Color(70, 0, 0), heatProportion));
}

void CharacterUIManager::ShowHeatDisplay(bool show)
{
  LOCK(weakText, text);

  text->SetEnabled(show);
}
