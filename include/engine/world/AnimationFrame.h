#ifndef __ANIMATION_FRAME__
#define __ANIMATION_FRAME__

#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include "Sprite.h"

class WorldObject;

class AnimationFrame
{
public:
  // Constructs an empty frame
  AnimationFrame(float duration);

  // Already adds a callback behavior
  AnimationFrame(std::function<void(WorldObject &)> callback, float duration);

  // Already adds a sprite behavior
  AnimationFrame(std::shared_ptr<Sprite> sprite, float duration);

  // Add a callback behavior
  void AddCallback(std::function<void(WorldObject &)> callback);

  // Add a sprite behavior (there can only be one at a time, a previous one will be overwritten)
  void SetSprite(std::shared_ptr<Sprite> sprite);

  // Get the frames's sprite
  std::shared_ptr<Sprite> GetSprite() const;

  // Triggers all of this frame's behaviors
  void Trigger(WorldObject &worldObject) const;

  // Get this frame's default duration
  float GetDuration() const;
  void SetDuration(float);

  // An offset to apply to the sprite renderer for this frame
  Vector2 spriteOffset;

private:
  // All the callbacks triggered by this frame
  std::vector<std::function<void(WorldObject &)>> callbacks;

  // A sprite to be set by this frame
  std::shared_ptr<Sprite> sprite;

  // The milliseconds it takes for this frame to finish
  float duration;
};

#endif