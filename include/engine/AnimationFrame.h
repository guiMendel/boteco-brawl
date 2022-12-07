#ifndef __ANIMATION_FRAME__
#define __ANIMATION_FRAME__

#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include "Sprite.h"

class GameObject;

class AnimationFrame
{
public:
  // Constructs an empty frame
  AnimationFrame(float duration);

  // Already adds a callback behavior
  AnimationFrame(std::function<void(GameObject &)> callback, float duration);

  // Already adds a sprite behavior
  AnimationFrame(std::shared_ptr<Sprite> sprite, float duration);

  // Add a callback behavior
  void AddCallback(std::function<void(GameObject &)> callback);

  // Add a sprite behavior (there can only be one at a time, a previous one will be overwritten)
  void SetSprite(std::shared_ptr<Sprite> sprite);

  // Get the frames's sprite
  std::shared_ptr<Sprite> GetSprite();

  // Triggers all of this frame's behaviors
  void Trigger(GameObject &gameObject) const;

  // Get this frame's default duration
  float GetDuration() const;

private:
  // All the callbacks triggered by this frame
  std::vector<std::function<void(GameObject &)>> callbacks;

  // A sprite to be set by this frame
  std::shared_ptr<Sprite> sprite;

  // The milliseconds it takes for this frame to finish
  float duration;
};

#endif