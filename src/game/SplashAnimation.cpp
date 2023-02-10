#include "SplashAnimation.h"
#include <functional>

using namespace std;
using namespace Helper;

struct SplashAnimation::Animation
{
  using Callback = function<void(float, Animation &)>;

  Animation(Callback callback, shared_ptr<Animation> next = nullptr, function<void(Animation &)> startCallback = nullptr)
      : callback(callback), startCallback(startCallback), next(next) {}

  Callback callback;
  function<void(Animation &)> startCallback;

  bool done{false};

  shared_ptr<Animation> next;
};

SplashAnimation::SplashAnimation(
    GameObject &associatedObject,
    shared_ptr<UIImage> splash,
    shared_ptr<UIImage> subtitle,
    shared_ptr<UIImage> prompt,
    shared_ptr<UIBackground> curtain,
    shared_ptr<ParticleEmitter> stompParticles)
    : UIComponent(associatedObject),
      weakSplash(splash),
      weakSubtitle(subtitle),
      weakPrompt(prompt),
      weakCurtain(curtain),
      weakStompParticles(stompParticles) {}

void SplashAnimation::Start()
{
  auto splashFallSpeed = make_shared<float>(0);
  auto splashFallAcceleration = make_shared<float>(15);

  // Slam splash art on screen
  auto slamSplash = [this, splashFallSpeed, splashFallAcceleration](float deltaTime, Animation &animation)
  {
    // Curtain fade in
    LOCK(weakCurtain, curtain);

    curtain->color.alpha = Color::ClampValid(curtain->color.alpha - 120 * deltaTime);

    // Splash fall
    LOCK(weakSplash, splash);

    splash->localScale = splash->localScale - Vector2::One() * (*splashFallSpeed) * deltaTime;

    // Accelerate
    (*splashFallSpeed) += (*splashFallAcceleration) * deltaTime;
    (*splashFallAcceleration) += 5 * deltaTime;

    if (splash->localScale.x <= 1 || splash->localScale.y <= 1)
    {
      splash->localScale = Vector2::One();
      curtain->color.alpha = 0;

      animation.done = true;
    }
  };

  auto slamEffect = [this](float, Animation &animation)
  {
    LOCK(weakSplash, splash);
    LOCK(weakStompParticles, stompParticles);

    Rectangle *box = dynamic_cast<Rectangle *>(stompParticles->origin.get());
    stompParticles->worldObject.SetPosition(splash->canvas.CanvasToWorld(splash->GetPosition()) +
                                            Vector2{box->width, box->height} / 2);

    stompParticles->StartEmission();

    animation.done = true;
  };

  auto delayDuration = make_shared<float>(3);

  auto delay = [this, delayDuration](float deltaTime, Animation &animation)
  {
    *delayDuration -= deltaTime;

    if (*delayDuration <= 0.5)
      Lock(weakCurtain)->color = Color::White();

    if (*delayDuration <= 0)
      animation.done = true;
  };

  // Oscillation speed
  auto oscillationSpeed = make_shared<float>(150);

  // Oscillation acceleration
  auto oscillationAcceleration = make_shared<float>(-150);

  // Oscillates the subtitle
  auto oscillateSubtitle = [this, oscillationSpeed, oscillationAcceleration](float deltaTime)
  {
    LOCK(weakSubtitle, subtitle);

    // Get current offset
    int currentOffset = subtitle->offset.y.AsRealPixels();

    // Apply speed
    *oscillationSpeed = *oscillationSpeed + *oscillationAcceleration * deltaTime;
    subtitle->offset.y.Set(UIDimension::RealPixels, currentOffset + *oscillationSpeed * deltaTime);

    // Ensure acceleration is pointing against offset
    if (GetSign(*oscillationAcceleration) == GetSign(subtitle->offset.y.AsRealPixels(), 0))
      *oscillationAcceleration = -*oscillationAcceleration;
  };

  auto flashFadeStart = [this](Animation &)
  {
    Lock(weakSubtitle)->SetEnabled(true);
  };

  auto minFlashDuration = make_shared<float>(3);

  auto flashFade = [this, minFlashDuration, oscillateSubtitle](float deltaTime, Animation &animation)
  {
    LOCK(weakCurtain, curtain);

    *minFlashDuration -= deltaTime;

    curtain->color.alpha = Color::ClampValid(curtain->color.alpha - 100 * deltaTime);

    // Apply subtitle oscillation
    oscillateSubtitle(deltaTime);

    if (curtain->color.alpha == 0 && *minFlashDuration <= 0)
      animation.done = true;
  };

  // Whether is fading prompt in or out
  auto fadeDirection = make_shared<float>(1);

  auto promptHighlight = [this, fadeDirection, oscillateSubtitle](float deltaTime, Animation &)
  {
    LOCK(weakPrompt, prompt);

    // Get current alpha of prompt
    int currentAlpha = prompt->style->imageColor.Get().alpha;

    prompt->style->imageColor.Set(Color(255, 255, 255,
                                        Color::ClampValid(currentAlpha + *fadeDirection * deltaTime * 220)));

    // Get new alpha
    int newAlpha = prompt->style->imageColor.Get().alpha;

    // Oscillate subtitle
    oscillateSubtitle(deltaTime);

    // Check if it's time to switch direction
    if (newAlpha == 0 || newAlpha == 255)
      *fadeDirection = -*fadeDirection;
  };

  auto promptAnimation = make_shared<Animation>(promptHighlight);
  auto flashAnimation = make_shared<Animation>(flashFade, promptAnimation, flashFadeStart);
  auto delayAnimation = make_shared<Animation>(delay, flashAnimation);
  auto slamEffectEnd = make_shared<Animation>(slamEffect, delayAnimation);
  auto slamAnimation = make_shared<Animation>(slamSplash, slamEffectEnd);

  currentAnimation = slamAnimation;
}

void SplashAnimation::Update(float deltaTime)
{
  if (currentAnimation == nullptr)
    return;

  if (currentAnimation->done)
  {
    currentAnimation = currentAnimation->next;

    if (currentAnimation && currentAnimation->startCallback)
      currentAnimation->startCallback(*currentAnimation);

    return;
  }

  currentAnimation->callback(deltaTime, *currentAnimation);
}
