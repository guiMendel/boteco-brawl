#ifndef __CHARACTER_LIFE_DISPLAY__
#define __CHARACTER_LIFE_DISPLAY__

#include "UIComponent.h"
#include "FallDeath.h"
#include "UIImage.h"
#include "UIText.h"
#include "UIContainer.h"

// Maps input from some source (player or AI) to the the creation and dispatch of the corresponding actions
class CharacterUIManager : public UIComponent
{
public:
  // How quickly the text size multiplier decays back to 1
  static const float textSizeMultiplierDecay;

  CharacterUIManager(
      GameObject &associatedObject,
      std::shared_ptr<FallDeath> fallDeath,
      std::shared_ptr<UIText> heatText,
      std::function<void()> addLifeIcon);
  virtual ~CharacterUIManager() {}

  void Start() override;
  void Update(float) override;

  // Set the the display text to the heat damage
  void UpdateHeatDisplay(float newHeat, float oldHeat);

  // Whether to show or hide heat display
  void ShowHeatDisplay(bool show);

private:
  void UpdateLifeCounter() const;

  // Erases all current lives
  void EraseLives() const;

  // Fall death component reference
  std::weak_ptr<FallDeath> weakFallDeath;

  // Adds an icon to represent how many lives character still has
  std::function<void()> addLifeIcon;

  // Container of life icons
  std::weak_ptr<UIContainer> weakLifeContainer;

  // Reference to text
  std::weak_ptr<UIText> weakText;

  // Original size of text
  int originalTextSize;

  // Current modifier applied to text size
  float textSizeModifier{1};

  // Color of the character's player
  Color playerColor;

  // Reference to object as container
  UIContainer &uiContainer;
};

#endif