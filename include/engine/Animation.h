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
  int startingFrame;

  // How many frames to be included in the slice
  // -1 means all through to the end
  int totalFrames;

  // Padding of image, in pixels
  int paddingLeft{0}, paddingTop{0}, paddingRight{0}, paddingBottom{0};

  // Gap between frames, in pixels
  int horizontalGap{0}, verticalGap{0};

  SpritesheetClipInfo(int width, int height, int totalFrames = -1, int startingFrame = 0)
      : width(width), height(height), startingFrame(startingFrame), totalFrames(totalFrames) {}
};

class Animation
{
  friend class Animator;

public:
  // Defines kinds of behavior allowed when cycle ends
  enum class CycleEndBehavior
  {
    // Repeat cycle
    Loop,
    // Play another animation
    PlayNext,
    // Play animator default animation
    PlayDefault,
  };

  // === EVENTS

  // Raised when the last frame finishes, even if is looping
  Event OnCycleEnd;

  // Raised when this animation stops playing (either interrupted or finished)
  Event OnStop;

  // === GENERAL PROPERTIES & METHODS

  bool operator==(Animation &other);
  bool operator!=(Animation &other);

  // Static id pool for all animations
  static int idGenerator;

  // Id of this animation
  int id{idGenerator++};

  // === ANIMATION SPECIFICITIES

  // Instances need their animator
  Animation(std::shared_ptr<Animator> animator);
  virtual ~Animation() {}

  // Access the animation name
  virtual std::string Name() = 0;

  // Behavior on cycle end
  virtual CycleEndBehavior &EndBehavior();

  // Get animation to play after this one ends (ignored if end behavior is not PlayNext)
  virtual std::shared_ptr<Animation> GetNext();

  // Playback speed modifier
  virtual float &SpeedModifier();

protected:
  // Provides an initial value to the frames
  virtual std::vector<AnimationFrame> InitializeFrames() = 0;

private:
  // Used in default implementation
  float speedModifier{1};

  // Used in default implementation
  CycleEndBehavior endBehavior{CycleEndBehavior::PlayDefault};

  // === FRAMES
public:
  // Each and every animation frame, to be played sequentially
  std::vector<AnimationFrame> &Frames();

  // Gets a specific frame
  AnimationFrame &GetFrame(int frame);

private:
  std::vector<AnimationFrame> frames;

  // === SEQUENCE CONTROL
protected:
  // Whether is currently playing
  bool IsPlaying();

  // Starts playing each frame sequentially, according to their duration and the playback speed
  void Start();

  // Stop animation
  void Stop();

  // Gets the next frame index
  int GetNextFrameIndex();

  // Concludes the current cycle and invokes the end behavior
  void Finish();

  // Called when animation starts
  virtual void InternalOnStart() {}

  // Called when animation stops for whatever reason
  virtual void InternalOnStop() {}

  // Checks (and resolves) whether need to advance a frame
  void Update(float deltaTime);

  // Sets the currently active frame
  void TriggerFrame(int frame);

  // How many seconds left to advance frame
  float secondsToNextFrame;

  // Currently active frame
  int currentFrame{0};

private:
  void InternalStart(bool raise);

  // === HELPERS
public:
  // Access frames directly
  AnimationFrame &operator[](int index);

  // Automatically generates the frame vector for a spritesheet animation
  // Allows offsetting the frames by some virtual pixels
  static std::vector<AnimationFrame> SliceSpritesheet(
      std::string filename, SpritesheetClipInfo clipInfo, float frameDuration, Vector2 virtualPixelOffset = Vector2::Zero(), SpriteConfig config = SpriteConfig());

  // Reference to it's animator
  std::weak_ptr<Animator> weakAnimator;
};

#endif