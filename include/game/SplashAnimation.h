#ifndef __SPLASH_ANIMATION__
#define __SPLASH_ANIMATION__

#include "UIComponent.h"
#include "UIImage.h"
#include "UIText.h"
#include "UIContainer.h"
#include "UIBackground.h"
#include "ParticleEmitter.h"

// Maps input from some source (player or AI) to the the creation and dispatch of the corresponding actions
class SplashAnimation : public UIComponent
{
public:
  // How quickly the text size multiplier decays back to 1
  static const float textSizeMultiplierDecay;

  SplashAnimation(
      GameObject &associatedObject,
      std::shared_ptr<UIContainer> mainContainer,
      std::shared_ptr<UIImage> splash,
      std::shared_ptr<UIImage> subtitle,
      std::shared_ptr<UIImage> prompt,
      std::shared_ptr<UIBackground> curtain,
      std::shared_ptr<ParticleEmitter> stompParticles);
  virtual ~SplashAnimation() {}

  void Start() override;
  void Update(float) override;

  // Pans the menu content to the given screen index
  void PanContent(int index);

  // Resets initial animation to it's final state
  void ResetInitialAnimation();

private:
  // Target index of screen
  int targetIndex{0};

  std::weak_ptr<UIContainer> weakMainContainer;
  std::weak_ptr<UIImage> weakSplash;
  std::weak_ptr<UIImage> weakSubtitle;
  std::weak_ptr<UIImage> weakPrompt;
  std::weak_ptr<UIBackground> weakCurtain;
  std::weak_ptr<ParticleEmitter> weakStompParticles;

  struct Animation;
  std::shared_ptr<Animation> currentAnimation;
  std::shared_ptr<Animation> promptAnimation;
};

#endif