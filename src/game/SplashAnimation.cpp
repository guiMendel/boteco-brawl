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
    shared_ptr<UIBackground> curtain,
    shared_ptr<ParticleEmitter> stompParticles)
    : UIComponent(associatedObject),
      weakSplash(splash),
      weakSubtitle(subtitle),
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

  auto slamEffectEnd = make_shared<Animation>(slamEffect);
  auto slamAnimation = make_shared<Animation>(slamSplash, slamEffectEnd);

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
