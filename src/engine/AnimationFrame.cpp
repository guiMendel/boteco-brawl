#include "AnimationFrame.h"
#include "GameObject.h"

using namespace std;

AnimationFrame::AnimationFrame(float duration)
    : duration(duration)
{
}

AnimationFrame::AnimationFrame(function<void(GameObject &)> callback, float duration)
    : AnimationFrame(duration)
{
  AddCallback(callback);
}

AnimationFrame::AnimationFrame(shared_ptr<Sprite> sprite, float duration)
    : AnimationFrame(duration)
{
  SetSprite(sprite);
}

void AnimationFrame::AddCallback(function<void(GameObject &)> callback)
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

void AnimationFrame::Trigger(GameObject &gameObject) const
{
  // Trigger the callbacks
  for (auto callback : callbacks)
    callback(gameObject);

  // Check if there is a sprite to set
  if (sprite != nullptr)
  {
    // Then the object must have a sprite renderer
    auto spriteRenderer = gameObject.RequireComponent<SpriteRenderer>();

    spriteRenderer->SetSprite(sprite);

    // Apply offset
    spriteRenderer->SetOffset(spriteOffset);
  }
}
