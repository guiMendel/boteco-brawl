#ifndef __ANIMATION_RECIPES__
#define __ANIMATION_RECIPES__

#define FIELD(Type, Name, initialValue) \
  Type &Name() override                 \
  {                                     \
    static Type field{initialValue};    \
    return field;                       \
  }

#define DEF_FRAMES(frames)                                \
  std::vector<AnimationFrame> InitializeFrames() override \
  {                                                       \
    return frames;                                        \
  }

#define DELCARE_FRAMES std::vector<AnimationFrame> InitializeFrames() override;

#define DEF_NAME(name) FIELD(std::string, Name, name)

#define DECLARE(Type, Name) Type &Name() override;

#include <memory>
#include "Animation.h"
#include "Circle.h"

class Animator;

namespace GeneralAnimations
{
  class AttackAnimation : public Animation
  {
  public:
    AttackAnimation(std::shared_ptr<Animator> animator) : Animation(animator) {}

    // Get damage & impulse for this attack
    // Default is is 0 for both
    virtual std::pair<float, Vector2> GetAttackProperties() const;

  protected:
    // Sets hitbox for a given frame
    void FrameHitbox(AnimationFrame &frame, std::vector<Circle> hitboxAreas = {});

  private:
    // Setup attack properties
    void InternalOnStart() override;
    void InternalOnStop() override;

    // Create attack child
    void SetupAttack();

    // Give attack child a hitbox
    void SetHitbox(const AnimationFrame &frame, std::vector<Circle> hitboxAreas);

    // Remove hitbox from attack child
    void RemoveHitbox();

    // Id of attack object
    int attackObjectId{-1};
  };

  class Run : public Animation
  {
  public:
    Run(std::shared_ptr<Animator> animator) : Animation(animator) {}

    DEF_NAME("run")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/Run.png", SpritesheetClipInfo(8, 8, 10), 0.1))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Idle : public Animation
  {
  public:
    Idle(std::shared_ptr<Animator> animator) : Animation(animator) {}

    DEF_NAME("idle")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/idle.png", SpritesheetClipInfo(8, 8), 0.1))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Jump : public Animation
  {
  public:
    Jump(std::shared_ptr<Animator> animator) : Animation(animator) {}

    DEF_NAME("jump")
    DELCARE_FRAMES
  };

  class Rise : public Animation
  {
  public:
    Rise(std::shared_ptr<Animator> animator) : Animation(animator) {}

    DEF_NAME("rise")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/jump.png", SpritesheetClipInfo(8, 8, 1, 2), 0.1))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Fall : public Animation
  {
  public:
    Fall(std::shared_ptr<Animator> animator) : Animation(animator) {}

    DEF_NAME("fall")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/jump.png", SpritesheetClipInfo(8, 8, 1, 3), 0.1))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Land : public Animation
  {
  public:
    Land(std::shared_ptr<Animator> animator) : Animation(animator) {}

    DEF_NAME("land")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/jump.png", SpritesheetClipInfo(8, 8, 1, 4), 0.1))
  };

  class Brake : public Animation
  {
  public:
    Brake(std::shared_ptr<Animator> animator) : Animation(animator) {}

    DEF_NAME("brake")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/carry.png", SpritesheetClipInfo(8, 8, 1, 1), 0.1))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Dash : public Animation
  {
  public:
    Dash(std::shared_ptr<Animator> animator) : Animation(animator) {}

    DEF_NAME("dash")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/carry.png", SpritesheetClipInfo(8, 8, 3, 5), 0.1))
  };

  class Neutral1 : public AttackAnimation
  {
  public:
    Neutral1(std::shared_ptr<Animator> animator) : AttackAnimation(animator) {}

    DEF_NAME("neutral1")
    DELCARE_FRAMES
  };

  class Neutral2 : public AttackAnimation
  {
  public:
    Neutral2(std::shared_ptr<Animator> animator) : AttackAnimation(animator) {}

    DEF_NAME("neutral2")
    DELCARE_FRAMES
  };

  class SpecialNeutral : public Animation
  {
  public:
    SpecialNeutral(std::shared_ptr<Animator> animator) : Animation(animator) {}

    DEF_NAME("specialNeutral")
    DELCARE_FRAMES
  };

}

#endif