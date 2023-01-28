#ifndef __CHARACTER_LIFE_DISPLAY__
#define __CHARACTER_LIFE_DISPLAY__

#include "Component.h"
#include "FallDeath.h"

// Maps input from some source (player or AI) to the the creation and dispatch of the corresponding actions
class CharacterLifeDisplay : public Component
{
public:
  CharacterLifeDisplay(GameObject &associatedObject, std::shared_ptr<FallDeath> fallDeath, float lifeSize, std::string spritePath);
  virtual ~CharacterLifeDisplay() {}

  void Start() override;

private:
  void UpdateDisplay() const;

  // Erases all current lives
  void EraseLives() const;

  std::weak_ptr<FallDeath> weakFallDeath;

  // Width of life sprite, in real pixels
  float lifeSizeRealPixels;

  std::string spritePath;

  Color color;
};

#endif