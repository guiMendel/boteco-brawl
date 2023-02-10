#ifndef __UI_INPUT_MANAGER__
#define __UI_INPUT_MANAGER__

#include "WorldComponent.h"
#include "SplashAnimation.h"

class MainMenuInput : public WorldComponent
{
public:
  MainMenuInput(GameObject &associatedObject, std::shared_ptr<SplashAnimation> animationHandler);
  virtual ~MainMenuInput() {}

  void Start() override;

private:
  // Triggered when start or enter is pressed
  void HitStart();

  std::weak_ptr<SplashAnimation> weakAnimationHandler;
};

#endif