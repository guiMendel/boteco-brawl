#ifndef __ARENA_UI_ANIMATION__
#define __ARENA_UI_ANIMATION__

#include "WorldComponent.h"
#include "UIBackground.h"
#include "UIContainer.h"
#include "UIText.h"

// Maps input from some source (player or AI) to the the creation and dispatch of the corresponding actions
class ArenaUIAnimation : public WorldComponent
{
public:
  ArenaUIAnimation(GameObject &associatedObject);
  virtual ~ArenaUIAnimation() {}

  void Update(float) override;

  // Display argument as victory text and transition back to initial scene
  void EndGame(std::string victoryText);

private:
  // Returns true if curtain is already faded all the way
  bool AnimateCurtain(float);

  // Returns true when countdown is over
  bool Countdown(float);

  // Slides in the victory text and transitions back to initial scene when done
  void AnimateVictoryText(float);

  // Current target x-padding of countdown container
  float targetPadding{0};

  // Target alpha of curtain
  int targetCurtainAlpha{0};

  // If end game was requested
  bool endGameRequested{false};

  std::weak_ptr<UIBackground> weakCurtain;
  std::weak_ptr<UIContainer> weakCountdown;
  std::weak_ptr<UIText> weakVictory;
};

#endif