#include "ArenaUIAnimation.h"
#include "MainScene.h"
#include "Arena.h"

using namespace std;

static const float curtainFadeSpeed{110};
static const float countdownSlideSpeed{250};
static const float countDownSlideInterval{0.7};

#define COUNTDOWN_SLIDE_TIMER "countdown-next-slide"

ArenaUIAnimation::ArenaUIAnimation(GameObject &associatedObject)
    : UIComponent(associatedObject), weakCurtain(GetScene()->RequireUIObject<UIContainer>(CURTAIN_OBJECT)->RequireComponent<UIBackground>())
{
}

void ArenaUIAnimation::Update(float deltaTime)
{
  if (FadeCurtain(deltaTime) == false)
    return;

  if (Countdown(deltaTime) == false)
    return;

  // Remove self on completion
  uiObject.RequestDestroy();
}

bool ArenaUIAnimation::FadeCurtain(float deltaTime)
{
  LOCK(weakCurtain, curtain);

  // Get next alpha of curtain
  auto nextAlpha = curtain->color.alpha - deltaTime * curtainFadeSpeed;

  // Detect if done
  if (nextAlpha <= 0)
  {
    curtain->color.alpha = 0;
    return true;
  }

  curtain->color.alpha = nextAlpha;

  return false;
}

bool ArenaUIAnimation::Countdown(float deltaTime)
{
  // Check if done
  if (targetPadding <= -500)
    return true;

  // Get current padding
  auto currentPadding = uiObject.padding.left.As(UIDimension::Percent);

  // If padding isn't on target
  if (currentPadding != targetPadding)
  {
    // Get next padding
    auto nextPadding = currentPadding - deltaTime * countdownSlideSpeed;

    // Check if will reach target
    if (nextPadding <= targetPadding)
    {
      nextPadding = targetPadding;

      // Start next slide timer
      uiObject.timer.Reset(COUNTDOWN_SLIDE_TIMER, -countDownSlideInterval);

      // Raise battle start on Rinha
      if (targetPadding <= -300)
        GetScene()->RequireFindComponent<Arena>()->OnBattleStart.Invoke();
    }

    uiObject.padding.left.Set(UIDimension::Percent, nextPadding);

    return false;
  }

  // Check if timer is up
  if (uiObject.timer.Get(COUNTDOWN_SLIDE_TIMER) >= 0)
  {
    uiObject.timer.Stop(COUNTDOWN_SLIDE_TIMER);

    // Increment target padding
    targetPadding -= 100;
  }

  return false;
}
