#include "Animation.h"
#include "Animator.h"
#include "Resources.h"
#include <iostream>

using namespace std;

Animation::Animation(string name, Animator &animator, vector<AnimationFrame> frames, bool loop)
    : name(name), frames(frames), loop(loop), animator(animator)
{
  Assert(name != "", "Invalid animation name: it can't be empty");
}

Animation::Animation(string name, Animator &animator, initializer_list<AnimationFrame> frames, bool loop)
    : Animation(name, animator, vector(frames), loop) {}

void Animation::Start()
{
  // Safecheck
  if (frames.empty())
  {
    cout << "WARNING: Tried playing animation with no frames!" << endl;
    return;
  }

  // Start with first frame
  SetFrame(0);
}

void Animation::Stop()
{
  OnStop.Invoke();
  currentFrame = 0;
}

void Animation::SetFrame(int frame)
{
  Assert((int)frames.size() > frame && frame >= 0, "Couldn't set animation frame: it was an invalid index");

  // Trigger it
  frames[frame].Trigger(animator.gameObject);

  // Get next frame time
  secondsToNextFrame = frames[frame].GetDuration() * speedModifier;

  currentFrame = frame;

  // Tell animator
  animator.IndicateCurrentFrame(frame);
}

bool Animation::IsPlaying() const
{
  return animator.currentAnimation == name;
}

void Animation::Update(float deltaTime)
{
  if (IsPlaying() == false)
    return;

  // Discount time
  secondsToNextFrame -= deltaTime;

  // Check if frame is up
  if (secondsToNextFrame > 0)
    return;

  // Get next frame
  int nextFrame = (currentFrame + 1) % frames.size();

  // If finished cycle
  if (nextFrame == 0)
  {
    // Announce
    OnCycleEnd.Invoke();

    // If has next
    if (auto nextAnimation = next.lock(); nextAnimation)
    {
      animator.Play(nextAnimation->name);
      return;
    }

    // If not looping
    if (loop == false)
    {
      // Transition to default
      if (transitionToDefault)
        animator.Play(animator.defaultAnimation);

      // Stop
      else
        animator.Stop();
      return;
    }
  }

  // Carry on to next frame
  SetFrame(nextFrame);
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

  return frames;
}

void Animation::SetNext(weak_ptr<Animation> next)
{
  this->next = next;
}
