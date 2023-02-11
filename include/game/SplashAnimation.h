#ifndef __SPLASH_ANIMATION__
#define __SPLASH_ANIMATION__

#include "UIComponent.h"
#include "UIImage.h"
#include "UIText.h"
#include "UIContainer.h"
#include "UIBackground.h"
#include "ParticleEmitter.h"

class MainMenuInput;

// Maps input from some source (player or AI) to the the creation and dispatch of the corresponding actions
class SplashAnimation : public UIComponent
{
  friend class MainMenuInput;

public:
  // How quickly the text size multiplier decays back to 1
  static const float textSizeMultiplierDecay;

  SplashAnimation(GameObject &associatedObject);
  virtual ~SplashAnimation() {}

  void Start() override;
  void Update(float) override;

  // Pans the menu content to the given screen index
  void PanContent(int index);

  // Resets initial animation to it's final state
  void ResetInitialAnimation();

private:
  // Plays current animation
  void PlayerCurrentAnimation(float deltaTime);

  // Pan screen
  void ApplyScreenPan(float deltaTime);

  // Raise bills
  void RaiseBills(float deltaTime);

  // Handles animations of start prompt
  void AnimateStartPrompt(float deltaTime);

  // Target index of screen
  int targetIndex{0};

  // Target offset of start prompt
  int targetStartPromptOffset{100};

  // Direction to change start prompt modulation color to
  int startPromptColorDirection{-1};

  std::weak_ptr<UIContainer> weakMainContainer;
  std::weak_ptr<UIImage> weakSplash;
  std::weak_ptr<UIImage> weakSubtitle;
  std::weak_ptr<UIImage> weakPrompt;
  std::weak_ptr<UIImage> weakStartPrompt;
  std::weak_ptr<UIBackground> weakCurtain;
  std::weak_ptr<ParticleEmitter> weakStompParticles;

  // Bills to animate removal of offset from
  std::vector<std::weak_ptr<UIObject>> raisingBills;

  struct Animation;
  std::shared_ptr<Animation> currentAnimation;
  std::shared_ptr<Animation> promptAnimation;
};

#endif