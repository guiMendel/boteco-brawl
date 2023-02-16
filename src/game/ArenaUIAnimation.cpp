#include "ArenaUIAnimation.h"
#include "ArenaScene.h"
#include "Arena.h"

using namespace std;

static const float curtainFadeSpeed{110};
// static const float countdownSlideSpeed{800};
// static const float countDownSlideInterval{0.1};
static const float countdownSlideSpeed{250};
static const float countDownSlideInterval{0.7};
static const float victoryShowTime{2};

#define COUNTDOWN_SLIDE_TIMER "countdown-next-slide"
#define VICTORY_SHOW_TIMER "victory-show"

#define SOUND_COUNT_1 "count-1"
#define SOUND_COUNT_2 "count-2"
#define SOUND_COUNT_3 "count-3"
#define SOUND_FIRE "gunshot"

const static vector countSounds{SOUND_COUNT_1, SOUND_COUNT_2, SOUND_COUNT_3, SOUND_FIRE};

ArenaUIAnimation::ArenaUIAnimation(GameObject &associatedObject)
    : WorldComponent(associatedObject),
      weakCurtain(GetScene()->RequireUIObject<UIContainer>(CURTAIN_OBJECT)->RequireComponent<UIBackground>()),
      weakCountdown(GetScene()->RequireUIObject<UIContainer>(COUNTDOWN_OBJECT)),
      weakVictory(GetScene()->RequireUIObject<UIText>(VICTORY_TEXT)),
      weakSound(Camera::GetMain()->worldObject.RequireComponent<Sound>())
{
  LOCK(weakSound, sound);

  sound->AddAudio(SOUND_COUNT_1, "./assets/sounds/battle/counting/count1.mp3");
  sound->AddAudio(SOUND_COUNT_2, "./assets/sounds/battle/counting/count2.mp3");
  sound->AddAudio(SOUND_COUNT_3, "./assets/sounds/battle/counting/count3.mp3");
  sound->AddAudio(SOUND_FIRE, "./assets/sounds/battle/counting/gunshot.mp3");
}

void ArenaUIAnimation::Update(float deltaTime)
{
  if (AnimateCurtain(deltaTime) == false)
    return;

  if (Countdown(deltaTime) == false)
    return;

  if (endGameRequested == false)
    return;

  AnimateVictoryText(deltaTime);
}

bool ArenaUIAnimation::AnimateCurtain(float deltaTime)
{
  LOCK(weakCurtain, curtain);

  // Get distance
  int distance = targetCurtainAlpha - curtain->color.alpha;

  // Get direction
  int curtainDirection = GetSign(distance);

  // Get the modification
  auto frameModification = deltaTime * curtainFadeSpeed * curtainDirection;

  // Detect if done
  if (abs(frameModification) >= abs(distance))
  {
    curtain->color.alpha = targetCurtainAlpha;
    return true;
  }

  curtain->color.alpha = curtain->color.alpha + frameModification;

  return false;
}

bool ArenaUIAnimation::Countdown(float deltaTime)
{
  IF_NOT_LOCK(weakCountdown, countdown)
  {
    return true;
  }

  // Check if done
  if (targetPadding <= -500)
  {
    // Destroy countdown
    countdown->RequestDestroy();

    return true;
  }

  // Get current padding
  auto currentPadding = countdown->padding.left.As(UIDimension::Percent);

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
      countdown->timer.Reset(COUNTDOWN_SLIDE_TIMER, -countDownSlideInterval);

      if (-nextPadding / 100 < countSounds.size())
        Lock(weakSound)->Play(countSounds[-nextPadding / 100]);

      // Raise battle start on Rinha
      if (targetPadding <= -300)
      {
        auto scene = RequirePointerCast<ArenaScene>(GetScene());

        scene->RequireFindComponent<Arena>()->OnBattleStart.Invoke();

        // Play music
        if (!musicStarted)
        {
          musicStarted = true;
          scene->music.Play("./assets/music/bgm3-Trio Virgulino - Forró do Rei (Forró Stream).mp3");
        }
      }
    }

    countdown->padding.left.Set(UIDimension::Percent, nextPadding);

    return false;
  }

  // Check if timer is up
  if (countdown->timer.Get(COUNTDOWN_SLIDE_TIMER) >= 0)
  {
    countdown->timer.Stop(COUNTDOWN_SLIDE_TIMER);

    // Increment target padding
    targetPadding -= 100;
  }

  return false;
}

void ArenaUIAnimation::EndGame(string victoryText)
{
  endGameRequested = true;
  GetScene()->RequireUIObject<UIText>(VICTORY_TEXT)->SetText(victoryText);
}

void ArenaUIAnimation::AnimateVictoryText(float deltaTime)
{
  auto victoryText = GetScene()->RequireUIObject<UIText>(VICTORY_TEXT);
  auto victoryContainer = victoryText->GetParent();

  // Get current margin
  auto currentMargin = victoryContainer->margin.left.As(UIDimension::Percent);

  // If margin isn't on target
  if (currentMargin != 0)
  {
    // Get next margin
    auto nextMargin = currentMargin - deltaTime * countdownSlideSpeed;

    // Check if will reach target
    if (nextMargin <= 0)
    {
      nextMargin = 0;

      // Start show timer
      victoryContainer->timer.Reset(VICTORY_SHOW_TIMER, -victoryShowTime);
    }

    victoryContainer->margin.left.Set(UIDimension::Percent, nextMargin);

    return;
  }

  // Check if timer is up
  if (victoryContainer->timer.Get(VICTORY_SHOW_TIMER) >= 0 && targetCurtainAlpha != 255)
  {
    victoryContainer->timer.Stop(VICTORY_SHOW_TIMER);

    // Close curtains
    targetCurtainAlpha = 255;

    return;
  }

  // If curtain closed, transition scene
  if (Lock(weakCurtain)->color.alpha != 255)
    return;

  auto &game = Game::GetInstance();

  game.SetScene(game.GetInitialScene());
}
