#ifndef __SPLASH_ANIMATION__
#define __SPLASH_ANIMATION__

#include "UIComponent.h"
#include "UIImage.h"
#include "UIText.h"
#include "UIContainer.h"
#include "UIBackground.h"

// Maps input from some source (player or AI) to the the creation and dispatch of the corresponding actions
class SplashAnimation : public UIComponent
{
public:
  // How quickly the text size multiplier decays back to 1
  static const float textSizeMultiplierDecay;

  SplashAnimation(
      GameObject &associatedObject,
      std::shared_ptr<UIImage> splash,
      std::shared_ptr<UIImage> subtitle,
      std::shared_ptr<UIBackground> curtain);
  virtual ~SplashAnimation() {}

  void Start() override;
  void Update(float) override;

private:
  std::weak_ptr<UIImage> weakSplash;
  std::weak_ptr<UIImage> weakSubtitle;
  std::weak_ptr<UIBackground> weakCurtain;

  struct Animation;
  std::shared_ptr<Animation> currentAnimation;
};

#endif