#ifndef __SPRITE_ANIMATOR_
#define __SPRITE_ANIMATOR_

#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include "GameObject.h"
#include "Component.h"
#include "SpriteRenderer.h"
#include "Vector2.h"
#include "Event.h"

class Animation;

class Animator : public Component
{
  friend class Animation;

public:
  typedef std::unordered_map<std::string, std::shared_ptr<Animation>> animation_map;
  typedef std::unordered_map<int, std::function<void()>> frame_callbacks;

  Animator(GameObject &associatedObject);

  virtual ~Animator() {}

  void Start() override;
  void Update(float deltaTime) override;

  // Adds an animation to the map
  // By convention, the first animation to be added is set as the initial animation
  void AddAnimation(std::shared_ptr<Animation> animation, bool makeInitial = false);

  // This one takes a recipe for an animation
  void AddAnimation(std::function<std::shared_ptr<Animation>(Animator &)> recipe, bool makeInitial = false);

  // Stops current animation and starts the given one
  void Play(std::string animation, std::function<void()> stopCallback = nullptr);

  // Stops current animation and starts the given one
  void Play(std::string animation, frame_callbacks callbacks, std::function<void()> stopCallback = nullptr);

  // Stops any animation
  void Stop();

  // If the current animation is this, stops it
  void Stop(std::string animation);

  Animation &GetAnimation(std::string name);

  std::string GetCurrentAnimation() const;

  // Triggered on animation cycle end
  Event OnCycleEnd;

  // Triggered when an animation has stopped
  Event OnAnimationStop;

  // Which animation to play on Start
  std::string defaultAnimation;

private:
  // Indicates to the animator which frame the playing animation is currently on
  void IndicateCurrentFrame(int frame);

  // Store animations
  animation_map animations;

  // Which animation is currently playing
  std::string currentAnimation{""};

  // Frame callbacks for current animation
  frame_callbacks currentAnimationCallbacks;
};

#endif