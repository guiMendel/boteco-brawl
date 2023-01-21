#ifndef __CHARACTER_BADGE__
#define __CHARACTER_BADGE__

#include "Component.h"
#include "Character.h"
#include "Text.h"
#include "Heat.h"

class CharacterBadge : public Component
{
public:
  CharacterBadge(GameObject &associatedObject, std::shared_ptr<Character> character);
  virtual ~CharacterBadge() {}

  void Start() override;

  // Set the heat damage to the display text
  void UpdateDisplay() const;

private:
  // Reference to character
  std::weak_ptr<Character> weakCharacter;

  // Reference to text
  std::weak_ptr<Text> weakText;
};

#endif