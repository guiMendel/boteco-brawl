#ifndef __SPRITE_ANIMATOR_
#define __SPRITE_ANIMATOR_

#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include "WorldObject.h"
#include "WorldComponent.h"
#include "SpriteRenderer.h"
#include "Vector2.h"
#include "Event.h"

class Animation;

class Animator : public WorldComponent
{
  friend class Animation;

public:
  // Type of function that returns an animation shared pointer
  typedef std::function<std::shared_ptr<Animation>()> animation_builder;
  typedef std::unordered_map<std::string, animation_builder> animation_map;

  Animator(GameObject &associatedObject);

  virtual ~Animator() {}

  void Start() override;
  void Update(float deltaTime) override;
  void OnBeforeDestroy() override;

  // Registers an animation to this animator
  template <class T, typename... Args>
  void RegisterAnimation(Args &&...args)
  {
    // Create it's builder
    auto builder = [this, args...]()
    {
      return std::make_shared<T>(*this, std::forward<Args>(args)...);
    };

    RegisterAnimation(builder);
  }

  // Stops current animation and starts the given one
  // This animation's type must have been previously registered to the animator
  // forceReset makes it so that the animation will start even if another animation of the same type is already playing
  void Play(std::shared_ptr<Animation> animation, bool forceReset = false);

  // Stops current animation and starts the given one
  // Receives the animation name, and it must be the name of a previously registered animation type
  void Play(std::string animation, bool forceReset = false);

  // Stops any animation
  void Stop(bool transitionToDefault = true);

  // If the current animation has this name, stops it
  void Stop(std::string animation);

  // Builds a new animation of this type's name
  std::shared_ptr<Animation> BuildAnimation(std::string name);

  // Checks if the animation's name is present
  bool HasAnimation(std::string name) const;

  // Get the currently active animation
  std::shared_ptr<Animation> GetCurrentAnimation() const;

  // Get the animation which is about to be played
  // Only non null during the small window between Start being called and the current animation being stopped
  std::shared_ptr<Animation> GetIncomingAnimation() const;

  // Triggered on animation cycle end
  Event OnCycleEnd;

  // Triggered when an animation has stopped
  Event OnAnimationStop;

  // Which animation to play on Start and when an animation transitions to default
  std::string defaultAnimation;

private:
  void RegisterAnimation(std::function<std::shared_ptr<Animation>()> animationBuilder, bool makeDefault = false);

  // Store registered animation types
  animation_map animations;

  // Which animation is currently playing
  std::shared_ptr<Animation> currentAnimation;

  // Animation which is about to be played
  std::shared_ptr<Animation> incomingAnimation;
};

#endif