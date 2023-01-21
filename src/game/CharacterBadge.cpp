#include "CharacterBadge.h"
#include <sstream>
#include <iomanip>

using namespace std;

CharacterBadge::CharacterBadge(GameObject &associatedObject, shared_ptr<Character> character)
    : Component(associatedObject), weakCharacter(character), weakText(gameObject.RequireComponent<Text>()) {}

void CharacterBadge::Start()
{
  gameObject.GetParent()->RequireComponent<Heat>()->OnTakeDamage.AddListener("update-display", [this](Damage)
                                                                { UpdateDisplay(); });

  UpdateDisplay();
}

void CharacterBadge::UpdateDisplay() const
{
  LOCK(weakText, text);
  auto heat = gameObject.GetParent()->RequireComponent<Heat>();

  // Convert heat to 1 decimal number string
  stringstream damageString;
  damageString << fixed << setprecision(1) << heat->GetHeat();

  // Set the text
  text->SetText(damageString.str());
}
