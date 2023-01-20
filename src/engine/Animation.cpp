#include "Animation.h"
#include "Animator.h"
#include "Resources.h"
#include <iostream>

using namespace std;

int Animation::idGenerator{0};

Animation::Animation(Animator &animator) : animator(animator)
{
  auto weakAnimator{weak_ptr(dynamic_pointer_cast<Animator>(animator.GetShared()))};

  // Link it's event types to this animation's
  OnCycleEnd.AddListener("animator-propagation", [this, weakAnimator]()
                         { LOCK(weakAnimator, animator); animator->OnCycleEnd.Invoke(); });

  OnStop.AddListener("animator-propagation", [this, weakAnimator]()
                     { LOCK(weakAnimator, animator); animator->OnAnimationStop.Invoke(); });
}

vector<AnimationFrame> &Animation::Frames()
{
  if (frames.empty())
    frames = InitializeFrames();

  return frames;
}

Animation::CycleEndBehavior &Animation::EndBehavior() { return endBehavior; }

float &Animation::SpeedModifier() { return speedModifier; }

shared_ptr<Animation> Animation::GetNext() { return nullptr; }

AnimationFrame &Animation::operator[](int index) { return GetFrame(index); }

void Animation::InternalStart(bool raise)
{
  if (raise)
    InternalOnStart();

  // Safecheck
  if (Frames().empty())
  {
    Assert(EndBehavior() != CycleEndBehavior::Loop, "Empty looping animations are forbidden");

    PassAndDelete();
    return;
  }

  // Start with first frame
  TriggerFrame(0);
}

void Animation::Start() { InternalStart(true); }

void Animation::Stop()
{
  InternalOnStop();
  OnStop.Invoke();
  currentFrame = 0;
}

AnimationFrame &Animation::GetFrame(int frame)
{
  Assert(frame >= 0 && frame < int(Frames().size()), "Invalid frame index");

  return Frames()[frame];
}

void Animation::TriggerFrame(int frame)
{
  currentFrame = frame;

  // Trigger it
  GetFrame(frame).Trigger(animator.gameObject);

  // Get next frame time
  secondsToNextFrame = GetFrame(frame).GetDuration() * speedModifier;
}

bool Animation::IsPlaying()
{
  return animator.GetCurrentAnimation()->Name() == Name();
}

void Animation::Update(float deltaTime)
{
  OnUpdate(deltaTime);

  if (IsPlaying() == false)
    return;

  if (finished)
  {
    PassAndDelete();
    return;
  }

  // Discount time
  secondsToNextFrame -= deltaTime;

  // Check if frame is up
  if (secondsToNextFrame > 0)
    return;

  // Get next frame
  int nextFrame{GetNextFrameIndex()};

  // If finished cycle
  if (nextFrame == 0)
  {
    PassAndDelete();
    return;
  }

  // Carry on to next frame
  TriggerFrame(nextFrame);
}

vector<AnimationFrame> Animation::SliceSpritesheet(string filename, SpritesheetClipInfo clipInfo, float frameDuration, Vector2 virtualPixelOffset, SpriteConfig config)
{
  // Get sprite
  auto sprite = Resources::GetSprite(filename);

  Assert(clipInfo.width > 0 && clipInfo.height > 0, "Invalid frame size");

  int discountedWidth = (sprite->GetUnscaledWidth() - clipInfo.paddingLeft - clipInfo.paddingRight);
  int discountedHeight = (sprite->GetUnscaledHeight() - clipInfo.paddingBottom - clipInfo.paddingTop);

  // Get frame count
  int columnFrameCount = discountedWidth /
                         (clipInfo.width + clipInfo.horizontalGap);

  int rowFrameCount = discountedHeight /
                      (clipInfo.height + clipInfo.verticalGap);

  Assert(columnFrameCount > 0 && rowFrameCount > 0, "Invalid padding values");

  //  See if one more fits without the extra gap (since the last one has no gap)
  if (discountedWidth - (clipInfo.width + clipInfo.horizontalGap) * columnFrameCount >= clipInfo.width)
    columnFrameCount++;

  if (discountedHeight - (clipInfo.height + clipInfo.verticalGap) * rowFrameCount >= clipInfo.height)
    rowFrameCount++;

  // Will hold the frames
  vector<AnimationFrame> frames;

  // Get initial frame coordinate
  int initialRow = clipInfo.startingFrame / columnFrameCount, initialColumn = clipInfo.startingFrame % columnFrameCount;

  // For each row
  for (int row = initialRow; row < rowFrameCount && frames.size() < (size_t)clipInfo.totalFrames; row++)
  {
    // For each column
    for (int column = initialColumn; column < columnFrameCount && frames.size() < (size_t)clipInfo.totalFrames; column++)
    {
      // Get this frame's clip
      SDL_Rect clip{column * (clipInfo.width + clipInfo.horizontalGap) + clipInfo.paddingLeft,
                    row * (clipInfo.height + clipInfo.verticalGap) + clipInfo.paddingTop,
                    clipInfo.width,
                    clipInfo.height};

      // Get sprite for this frame
      auto sprite = Resources::GetSprite(filename, config, clip);

      // Build the frame
      auto frame = AnimationFrame(sprite, frameDuration);

      // Apply the offset
      if (virtualPixelOffset)
        frame.spriteOffset = virtualPixelOffset / Game::defaultVirtualPixelsPerUnit;

      frames.push_back(frame);
    }
  }

  Assert(frames.empty() == false, "Failed to slice spritesheet");

  return frames;
}

int Animation::GetNextFrameIndex() { return (currentFrame + 1) % Frames().size(); }

void Animation::PassAndDelete()
{
  // Announce
  OnCycleEnd.Invoke();

  // If loops, simply carry on
  if (EndBehavior() == CycleEndBehavior::Loop)
  {
    InternalStart(false);
    return;
  }

  // Get next
  if (EndBehavior() == CycleEndBehavior::PlayNext)
    animator.Play(GetNext());

  // Otherwise, transitions to default
  else
    animator.Play(animator.defaultAnimation);
}

bool Animation::operator==(Animation &other)
{
  return Name() == other.Name();
}

bool Animation::operator!=(Animation &other) { return !(*this == other); }

void Animation::Finish() { finished = true; }
