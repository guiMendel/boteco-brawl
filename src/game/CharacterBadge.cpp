#include "CharacterBadge.h"
#include "FallOffDeath.h"
#include <sstream>
#include <iomanip>

using namespace std;

const float CharacterBadge::textSizeMultiplierDecay{3};

CharacterBadge::CharacterBadge(GameObject &associatedObject, shared_ptr<Character> character)
    : Component(associatedObject), weakCharacter(character), weakText(gameObject.RequireComponent<Text>())
{
  LOCK(weakText, text);
  originalTextSize = text->GetFontSize();
}

void CharacterBadge::Start()
{
  gameObject.GetParent()->RequireComponent<Heat>()->OnTakeDamage.AddListener("update-heat-display", [this](Damage damage)
                                                                             { UpdateDisplay(damage.heatDamage); });

  gameObject.GetParent()->RequireComponent<FallOffDeath>()->OnDeath.AddListener("update-heat-display", [this]()
                                                                             { UpdateDisplay(0); });

  UpdateDisplay(0);
}

void CharacterBadge::Update(float deltaTime)
{
  if (textSizeModifier != 1)
  {
    if (textSizeModifier > 1)
      textSizeModifier = max(1.0f, textSizeModifier - textSizeMultiplierDecay * deltaTime);
    else
      textSizeModifier = min(1.0f, textSizeModifier + textSizeMultiplierDecay * deltaTime);

    // Apply new multiplier
    LOCK(weakText, text);

    text->SetFontSize(round(originalTextSize * textSizeModifier));
  }
}

void CharacterBadge::UpdateDisplay(float damage)
{
  LOCK(weakText, text);
  auto heat = gameObject.GetParent()->RequireComponent<Heat>();

  // Convert heat to 1 decimal number string
  stringstream damageString;
  damageString << fixed << setprecision(1) << heat->GetHeat();

  // Set the text
  text->SetText(damageString.str());

  // Give it an appropriate render order
  text->renderOrder = SDL_GetTicks();

  // Set a size pop effect
  textSizeModifier = min(textSizeModifier + log10f(damage + 5), 5.0f);

  // Give it a color proportional to heat
  float heatProportion = heat->GetHeat() / 100.0f;
  text->SetColor(Lerp(Color::White(), Color::Red(), heatProportion));
  text->SetBorderColor(Lerp(Color::Black(), Color(70, 0, 0), heatProportion));
}
