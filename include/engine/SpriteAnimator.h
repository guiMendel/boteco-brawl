#ifndef __SPRITE_ANIMATOR_
#define __SPRITE_ANIMATOR_

#include <memory>
#include "GameObject.h"
#include "Component.h"
#include "Sprite.h"
#include "Vector2.h"
#include "Event.h"

class SpriteAnimator : public Component
{
public:
  SpriteAnimator(GameObject &associatedObject, std::weak_ptr<Sprite> sprite, Vector2 frameDimensions, float secondsPerFrame, bool loop = false)
      : Component(associatedObject), loop(loop), spriteWeak(sprite), frameDimensions(frameDimensions), secondsPerFrame(secondsPerFrame)
  {
    ConfigureSpriteFrames();
  }

  virtual ~SpriteAnimator() {}
  
  void Update(float deltaTime) override;

  // Get how many frames the animation has
  int GetFrameCount() { return rowFrameCount * columnFrameCount; }

  // Get current frame
  int GetCurrentFrame() { return currentFrame; }

  // Set the current frame
  void SetFrame(int frameIndex);

  float GetFrameWidth() { return frameDimensions.x; }
  float GetFrameHeight() { return frameDimensions.y; }

  // Triggered on animation cycle end
  Event OnCycleEnd;

  // Whether to loop
  bool loop{false};

private:
  void ConfigureSpriteFrames();

  // The current frame index
  int currentFrame{0};

  // How many frames the sprite has in each row
  int rowFrameCount;

  // How many frames the sprite has in each column
  int columnFrameCount;

  // How much time has passed since the last frame update
  float frameElapsedTime{0};

  // Pointer to sprite component
  std::weak_ptr<Sprite> spriteWeak;

  // Dimensions of the frames to clip from the sprite
  Vector2 frameDimensions;

  // Time to spend in each frame, in seconds
  float secondsPerFrame;

  // Whether animation is playing
  bool playing{true};
};

#endif