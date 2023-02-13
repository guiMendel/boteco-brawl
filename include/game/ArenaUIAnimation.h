#ifndef __ARENA_UI_ANIMATION__
#define __ARENA_UI_ANIMATION__

#include "UIComponent.h"
#include "UIBackground.h"
#include "UIContainer.h"

// Maps input from some source (player or AI) to the the creation and dispatch of the corresponding actions
class ArenaUIAnimation : public UIComponent
{
public:
  ArenaUIAnimation(GameObject &associatedObject);
  virtual ~ArenaUIAnimation() {}

  void Update(float) override;

private:
  // Returns true if curtain is already faded all the way
  bool FadeCurtain(float);

  // Returns true when countdown is over
  bool Countdown(float);

  // Current target x-padding of countdown container
  float targetPadding{0};

  std::weak_ptr<UIBackground> weakCurtain;
};

#endif