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
  Animation(std::string name, Animator &animator, std::initializer_list<AnimationFrame> frames);

  // Initialize with frame vector
  Animation(std::string name, Animator &animator, std::vector<AnimationFrame> frames);

  void SetNext(std::weak_ptr<Animation> next);

  AnimationFrame &operator[](int index) { return frames[index]; }
  AnimationFrame operator[](int index) const { return frames[index]; }

  // Automatically generates the frame vector for a spritesheet animation
  static std::vector<AnimationFrame> SliceSpritesheet(
      std::string filename, SpritesheetClipInfo clipInfo, float frameDuration, SpriteConfig config = SpriteConfig());

  Event OnCycleEnd;
  Event OnStop;

  // It's identifier
  const std::string name;

  // Each and every animation frame, to be played sequentially
  std::vector<AnimationFrame> frames;

  // Playback speed modifier
  float speedModifier{1};

  // Whether to loop
  bool loop{false};

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