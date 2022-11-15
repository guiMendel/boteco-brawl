#include "SpriteAnimator.h"

void SpriteAnimator::ConfigureSpriteFrames()
{
  // Get sprite ref
  auto sprite = spriteWeak.lock();

  if (!sprite)
  {
    enabled = false;
    return;
  }

  // Get frame count
  columnFrameCount = sprite->GetUnscaledWidth() / frameDimensions.x;
  rowFrameCount = sprite->GetUnscaledHeight() / frameDimensions.y;

  // Set first frame
  SetFrame(0);
}

void SpriteAnimator::Update(float deltaTime)
{
  if (playing == false)
    return;

  // Count this frame's time
  frameElapsedTime += deltaTime;

  // Do nothing if timer isn't up
  if (frameElapsedTime < secondsPerFrame)
    return;

  // If this is the cycle's last frame
  if (currentFrame == GetFrameCount() - 1)
  {
    // Trigger event
    OnCycleEnd.Invoke();

    // Stop playing if not supposed to loop
    if (loop == false)
    {
      playing = false;
      return;
    }
  }

  // Advance frame
  SetFrame(currentFrame + 1);
}

void SpriteAnimator::SetFrame(int frameIndex)
{
  Assert(frameIndex >= 0, "Invalid frame index");

  // Get sprite
  auto sprite = spriteWeak.lock();

  int frameCount = GetFrameCount();

  if (!sprite)
  {
    enabled = false;
    return;
  }

  // Reset time
  frameElapsedTime = 0;

  // Update to this frame
  // Limit frame index to frame space
  currentFrame = frameIndex % frameCount;

  // Finit's row
  int frameRow = currentFrame / columnFrameCount;

  // And it's column
  int frameColumn = currentFrame % columnFrameCount;

  // Set the sprite's clip
  sprite->SetClip(
      frameColumn * frameDimensions.x,
      frameRow * frameDimensions.y,
      frameDimensions.x,
      frameDimensions.y);
}
