#include "SplashAnimation.h"
#include <functional>

using namespace std;

struct SplashAnimation::Animation
{
  using Callback = function<void(float, Animation &)>;

  Animation(Callback callback, shared_ptr<Animation> next = nullptr)
      : callback(callback), next(next) {}

  Callback callback;

  bool done{false};

  shared_ptr<Animation> next;
};

SplashAnimation::SplashAnimation(
    GameObject &associatedObject,
    shared_ptr<UIImage> splash,
    shared_ptr<UIImage> subtitle,
    shared_ptr<UIBackground> curtain)
    : UIComponent(associatedObject),
      weakSplash(splash),
      weakSubtitle(subtitle),
      weakCurtain(curtain) {}

void SplashAnimation::Start()
{
  cout << "Start it" << endl;

  auto splashFallSpeed = make_shared<float>(0.1);
  auto splashFallAcceleration = make_shared<float>(1);

  // Slam splash art on screen
  auto slamSplash = [this, splashFallSpeed, splashFallAcceleration](float deltaTime, Animation &animation)
  {
    // Curtain fade in
    LOCK(weakCurtain, curtain);

    curtain->color.alpha = Color::ClampValid(curtain->color.alpha - 60 * deltaTime);

    // Splash fall
    LOCK(weakSplash, splash);

    splash->localScale = splash->localScale - Vector2::One() * (*splashFallSpeed) * deltaTime;

    // Accelerate
    (*splashFallSpeed) += (*splashFallAcceleration) * deltaTime;
    (*splashFallAcceleration) += 5 * deltaTime;

    if (splash->localScale.x <= 1 || splash->localScale.y <= 1)
    {
      splash->localScale = Vector2::One();

      if (curtain->color.alpha == 0)
        animation.done = true;
    }
  };

  auto slamAnimation = make_shared<Animation>(slamSplash);

  currentAnimation = slamAnimation;
}

void SplashAnimation::Update(float deltaTime)
{
  // cout << "Up it" << endl;

  if (currentAnimation == nullptr)
    return;

  if (currentAnimation->done)
  {
    currentAnimation = currentAnimation->next;
    return;
  }

  currentAnimation->callback(deltaTime, *currentAnimation);
}
