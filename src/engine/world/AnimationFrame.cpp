#include "AnimationFrame.h"
#include "WorldObject.h"

using namespace std;

AnimationFrame::AnimationFrame(float duration)
    : duration(duration)
{
}

AnimationFrame::AnimationFrame(function<void(WorldObject &)> callback, float duration)
    : AnimationFrame(duration)
{
  AddCallback(callback);
}

AnimationFrame::AnimationFrame(shared_ptr<Sprite> sprite, float duration)
    : AnimationFrame(duration)
{
  SetSprite(sprite);
}

void AnimationFrame::AddCallback(function<void(WorldObject &)> callback)
{
  callbacks.push_back(callback);
}

void AnimationFrame::SetSprite(shared_ptr<Sprite> sprite)
{
  this->sprite = sprite;
}

shared_ptr<Sprite> AnimationFrame::GetSprite() const
{
  return sprite;
}

float AnimationFrame::GetDuration() const
{
  return duration;
}

void AnimationFrame::SetDuration(float value) { duration = value; }

void AnimationFrame::Trigger(WorldObject &worldObject) const
{
  // Trigger the callbacks
  for (auto callback : callbacks)
    callback(worldObject);

  // Check if there is a sprite to set
  if (sprite != nullptr)
  {
    // Then the object must have a sprite renderer
    auto spriteRenderer = worldObject.RequireComponent<SpriteRenderer>();

    spriteRenderer->SetSprite(sprite);

    // Apply offset
    spriteRenderer->SetOffset(spriteOffset);
  }
}
