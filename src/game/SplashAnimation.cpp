#include "SplashAnimation.h"
#include "MenuScene.h"
#include <functional>

using namespace std;
using namespace Helper;

// Speed with which to raise bills, in percent per second
static const float raiseBillSpeed{120};

// Speed with which to slide start prompt
static const float startPromptSlideSpeed{200};

// Min start prompt color modulation
static const int startPromptMinColor{200};

// Speed to change prompt color modulation
static const int startPromptModulateSpeed{70};

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

SplashAnimation::SplashAnimation(GameObject &associatedObject)
    : UIComponent(associatedObject),
      weakMainContainer(GetScene()->RequireUIObject<UIContainer>(MAIN_CONTAINER_OBJECT)),
      weakSplash(GetScene()->RequireUIObject<UIImage>(SPLASH_OBJECT)),
      weakSubtitle(GetScene()->RequireUIObject<UIImage>(SUBTITLE_OBJECT)),
      weakPrompt(GetScene()->RequireUIObject<UIImage>(START_PROMPT_OBJECT)),
      weakStartPrompt(GetScene()->RequireUIObject<UIImage>(START_ARENA_IMAGE)),
      weakCurtain(GetScene()->RequireUIObject<UIContainer>(CURTAIN_OBJECT)->RequireComponent<UIBackground>()),
      weakStompParticles(GetScene()->RequireWorldObject(PARTICLES_OBJECT)->RequireComponent<ParticleEmitter>()) {}

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

  auto slamEffectIntensity = make_shared<float>(40);

  auto slamEffect = [this, slamEffectIntensity](float deltaTime, Animation &animation)
  {
    LOCK(weakMainContainer, mainContainer);

    // Take random offset
    Vector2 newOffset = Vector2::Angled(RandomRange(0, 2 * M_PI), RandomRange(*slamEffectIntensity / 2, *slamEffectIntensity));

    // Apply it
    mainContainer->offset.Set(UIDimension::RealPixels, newOffset);

    // Reduce intensity
    if ((*slamEffectIntensity -= 70 * deltaTime) <= 0)
      animation.done = true;
  };

  auto slamEffectStart = [this, slamEffectIntensity](Animation &)
  {
    LOCK(weakSplash, splash);
    LOCK(weakStompParticles, stompParticles);

    Rectangle *box = dynamic_cast<Rectangle *>(stompParticles->origin.get());
    stompParticles->worldObject.SetPosition(splash->canvas.CanvasToWorld(splash->GetPosition()) +
                                            Vector2{box->width, box->height} / 2);

    stompParticles->StartEmission();
  };

  auto delayDuration = make_shared<float>(2);

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
    // Turn on subtitle
    Lock(weakSubtitle)->SetEnabled(true);

    // Change splash image to the sliced version
    Lock(weakSplash)->SetImagePath("./assets/images/splash-screen/splash.png");
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

  promptAnimation = make_shared<Animation>(promptHighlight);
  auto flashAnimation = make_shared<Animation>(flashFade, promptAnimation, flashFadeStart);
  auto delayAnimation = make_shared<Animation>(delay, flashAnimation);
  auto slamEffectEnd = make_shared<Animation>(slamEffect, delayAnimation, slamEffectStart);
  auto slamAnimation = make_shared<Animation>(slamSplash, slamEffectEnd);

  currentAnimation = slamAnimation;
}

void SplashAnimation::Update(float deltaTime)
{
  PlayerCurrentAnimation(deltaTime);

  ApplyScreenPan(deltaTime);

  RaiseBills(deltaTime);

  AnimateStartPrompt(deltaTime);
}

void SplashAnimation::PanContent(int index)
{
  targetIndex = index;
}

void SplashAnimation::ResetInitialAnimation()
{
  LOCK(weakCurtain, curtain);
  LOCK(weakSplash, splash);

  curtain->color.alpha = 0;
  splash->localScale = Vector2::One();
  Lock(weakSubtitle)->SetEnabled(true);
  Lock(weakMainContainer)->offset.Set(UIDimension::None);
  Lock(weakSplash)->SetImagePath("./assets/images/splash-screen/splash.png");

  currentAnimation = promptAnimation;
}

void SplashAnimation::PlayerCurrentAnimation(float deltaTime)
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

void SplashAnimation::ApplyScreenPan(float deltaTime)
{
  LOCK(weakMainContainer, mainContainer);

  // Get target padding
  int targetPadding = targetIndex * -100;
  int currentPadding = mainContainer->padding.top.As(UIDimension::Percent);

  if (currentPadding != targetPadding)
  {
    int paddingDifference = targetPadding - currentPadding;

    currentPadding += min(int(deltaTime * 100), abs(paddingDifference)) * GetSign(paddingDifference);

    mainContainer->padding.top.Set(UIDimension::Percent, currentPadding);
  }
}

void SplashAnimation::RaiseBills(float deltaTime)
{
  // For each bill
  auto billIterator = raisingBills.begin();
  while (billIterator != raisingBills.end())
  {
    auto bill = Lock(*billIterator);

    // Get new offset
    float newOffset = bill->offset.y.As(UIDimension::Percent) - raiseBillSpeed * deltaTime;

    // Detect animation end
    if (newOffset <= 0)
    {
      // Remove all offset
      bill->offset.Set(UIDimension::None);

      // Remove this entry
      billIterator = raisingBills.erase(billIterator);
    }

    // Otherwise
    else
    {
      // Apply raise
      bill->offset.y.Set(UIDimension::Percent, newOffset);

      billIterator++;
    }
  }
}

void SplashAnimation::AnimateStartPrompt(float deltaTime)
{
  LOCK(weakStartPrompt, startPrompt);

  // Get it's current offset
  auto currentOffset = startPrompt->offset.x.As(UIDimension::Percent);

  // cout << "currentOffset: " << currentOffset << ", targetStartPromptOffset: " << targetStartPromptOffset << endl;

  // If it's different from target
  if (currentOffset != targetStartPromptOffset)
  {
    // Get distance
    auto distance = targetStartPromptOffset - currentOffset;
    auto frameDisplacement = GetSign(distance) * startPromptSlideSpeed * deltaTime;

    if (abs(frameDisplacement) < abs(distance))
      startPrompt->offset.x.Set(UIDimension::Percent, currentOffset + frameDisplacement);
    else
      startPrompt->offset.x.Set(UIDimension::Percent, targetStartPromptOffset);
  }

  // Modulate the color
  auto newColor = Clamp(
      int(startPrompt->style->imageColor.Get().red + startPromptColorDirection * startPromptModulateSpeed * deltaTime),
      startPromptMinColor,
      255);

  // Change direction if necessary
  if (newColor == startPromptMinColor || newColor == 255)
    startPromptColorDirection = -startPromptColorDirection;

  // Set the new color
  startPrompt->style->imageColor.Set(Color(newColor, newColor, newColor));
}

void SplashAnimation::TransitionOutAndExecute(std::function<void()> callback)
{
  // Set black camera background
  Camera::GetMain()->background = Color::Black();

  // Hide animation views
  GetScene()->RequireWorldObject(IDLE_ANIMATIONS_OBJECT)->SetEnabled(false);

  // Transitions menu up and leaves screen black
  auto transitionOut = [this](float deltaTime, Animation &animation)
  {
    LOCK(weakMainContainer, mainContainer);

    // Get new margin
    int newOffset = mainContainer->offset.y.As(UIDimension::Percent) - 120 * deltaTime;

    // Check if done
    if (newOffset <= -150)
    {
      mainContainer->offset.y.Set(UIDimension::Percent, -150);
      animation.done = true;
    }
    else
      mainContainer->offset.y.Set(UIDimension::Percent, newOffset);
  };

  // Executes the callback
  auto executeCallback = [callback](float, Animation &animation)
  {
    callback();
    animation.done = true;
  };

  // Make the animations
  auto callbackAnimation = make_shared<Animation>(executeCallback);
  auto transitionAnimation = make_shared<Animation>(transitionOut, callbackAnimation);

  // Play it
  currentAnimation = transitionAnimation;
}
