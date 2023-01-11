#ifndef __ANIMATION__
#define __ANIMATION__

#include "AnimationFrame.h"
#include "Event.h"
#include <string>

class Animator;

struct SpritesheetClipInfo
{
  // Width of a frame, in pixels
  int width;

  // Height of a frame, in pixels
  int height;

  // First frame index to be included in the slice
  int startingFrame{0};

  // How many frames to be included in the slice
  // -1 means all through to the end
  int totalFrames;

  // Padding of image, in pixels
  int paddingLeft{0}, paddingTop{0}, paddingRight{0}, paddingBottom{0};

  // Gap between frames, in pixels
  int horizontalGap{0}, verticalGap{0};

  SpritesheetClipInfo(int width, int height, int totalFrames = -1)
      : width(width), height(height), totalFrames(totalFrames) {}
};

class Animation
{
  friend class Animator;

public:
  // Initialize manually
  Animation(std::string name, Animator &animator, std::initializer_list<AnimationFrame> frames, bool loop = false);

  // Initialize with frame vector
  Animation(std::string name, Animator &animator, std::vector<AnimationFrame> frames, bool loop = false);

  void SetNext(std::weak_ptr<Animation> next);

  AnimationFrame &operator[](int index) { return frames[index]; }
  AnimationFrame operator[](int index) const { return frames[index]; }

  // Automatically generates the frame vector for a spritesheet animation
  // Allows offsetting the frame's by some virtual pixels
  static std::vector<AnimationFrame> SliceSpritesheet(
      std::string filename, SpritesheetClipInfo clipInfo, float frameDuration, Vector2 virtualPixelOffset = Vector2::Zero(), SpriteConfig config = SpriteConfig());

  // Raised when the last frame finishes, even if is looping
  Event OnCycleEnd;

  // Raised when this animation stops playing (either interrupted or finished)
  Event OnStop;

  // It's name
  const std::string name;

  // Each and every animation frame, to be played sequentially
  std::vector<AnimationFrame> frames;

  // Playback speed modifier
  float speedModifier{1};

  // Whether to loop
  bool loop{false};

  // Whether to transition to animator default animation after finish (ignored if looping)
  bool transitionToDefault{true};

protected:
  // Starts playing each frame sequentially, according to their duration and the playback speed
  void Start();

  void Stop();

  // Checks (and resolves) whether need to advance a frame
  void Update(float deltaTime);

  // Sets the current frame
  void SetFrame(int frame);

  // Whether is currently playing
  bool IsPlaying() const;

  // How many seconds left to advance frame
  float secondsToNextFrame;

  // Current frame
  int currentFrame{0};

  // Reference to it's animator
  Animator &animator;

  // Animation to play on this one`s end (overrules looping)
  std::weak_ptr<Animation> next;
};

#endif