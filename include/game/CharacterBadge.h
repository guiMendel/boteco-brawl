#ifndef __CHARACTER_BADGE__
#define __CHARACTER_BADGE__

#include "Component.h"
#include "Character.h"
#include "Text.h"
#include "Heat.h"

class CharacterBadge : public Component
{
public:
  // How quickly the text size multiplier decays back to 1
  static const float textSizeMultiplierDecay;

  CharacterBadge(WorldObject &associatedObject, std::shared_ptr<Character> character);
  virtual ~CharacterBadge() {}

  void Start() override;
  void Update(float) override;

  // Set the heat damage to the display text
  void UpdateDisplay(float newHeat, float oldHeat);

  void ShowBadge(bool show);

private:
  // Reference to character
  std::weak_ptr<Character> weakCharacter;

  // Reference to text
  std::weak_ptr<Text> weakText;

  // Original size of text
  int originalTextSize;

  // Current modifier applied to text size
  float textSizeModifier{1};
};

#endif