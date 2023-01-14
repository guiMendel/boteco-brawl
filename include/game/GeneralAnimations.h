#ifndef __ANIMATION_RECIPES__
#define __ANIMATION_RECIPES__

#include <memory>
#include "StatefulAnimation.h"
#include "Circle.h"

#define CONSTRUCTOR_AND_DESTRUCTOR_WITH_PARENT(ClassName, Parent)  \
  ClassName(std::shared_ptr<Animator> animator) : Parent(animator) \
  {                                                                \
  }                                                                \
  virtual ~ClassName()                                             \
  {                                                                \
  }

#define CONSTRUCTOR_AND_DESTRUCTOR(ClassName) CONSTRUCTOR_AND_DESTRUCTOR_WITH_PARENT(ClassName, StatefulAnimation)
#define ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(ClassName) CONSTRUCTOR_AND_DESTRUCTOR_WITH_PARENT(ClassName, AttackAnimation)

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

#define SET_DAMAGE(damage, impulse, stunTime)                            \
  std::tuple<float, Vector2, float> GetAttackProperties() const override \
  {                                                                      \
    return {damage, impulse, stunTime};                                  \
  }

#define ATTACK_SEQUENCE(frame)           \
  int OpenSequenceFrame() const override \
  {                                      \
    return frame;                        \
  }

#define ATTACK_CANCEL(frame)       \
  int CancelFrame() const override \
  {                                \
    return frame;                  \
  }

class Animator;

namespace GeneralAnimations
{
  class AttackAnimation : public StatefulAnimation
  {
  public:
    AttackAnimation(std::shared_ptr<Animator> animator) : StatefulAnimation(animator) {}
    virtual ~AttackAnimation() {}

    // Get damage & impulse for this attack
    // Default is is 0 for both
    virtual std::tuple<float, Vector2, float> GetAttackProperties() const;

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

  class Run : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Run)

    DEF_NAME("run")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/Run.png", SpritesheetClipInfo(8, 8, 10), 0.1))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Idle : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Idle)

    DEF_NAME("idle")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/idle.png", SpritesheetClipInfo(8, 8), 0.1))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Jump : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Jump)

    DEF_NAME("jump")
    DELCARE_FRAMES
  };

  class Rise : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Rise)

    DEF_NAME("rise")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/jump.png", SpritesheetClipInfo(8, 8, 1, 2), 0.1))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Fall : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Fall)

    DEF_NAME("fall")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/jump.png", SpritesheetClipInfo(8, 8, 1, 3), 0.1))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Land : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Land)

    DEF_NAME("land")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/jump.png", SpritesheetClipInfo(8, 8, 1, 4), 0.1))
  };

  class Brake : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Brake)

    DEF_NAME("brake")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/carry.png", SpritesheetClipInfo(8, 8, 1, 1), 0.1))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Ouch1 : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Ouch1)

    DEF_NAME("ouch1")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/ouch.png", SpritesheetClipInfo(8, 8, 1), 0.1))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Ouch2 : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Ouch2)

    DEF_NAME("ouch2")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/ouch.png", SpritesheetClipInfo(8, 8, 1, 1), 0.1))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Dash : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Dash)

    DEF_NAME("dash")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/carry.png", SpritesheetClipInfo(8, 8, 3, 5), 0.1))
  };

  class Neutral1 : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(Neutral1)

    DEF_NAME("neutral1")
    DELCARE_FRAMES

    SET_DAMAGE(1, Vector2::Angled(Helper::DegreesToRadians(-5), 0.5), 0.2)

    ATTACK_SEQUENCE(4)
    ATTACK_CANCEL(5)
  };

  class Neutral2 : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(Neutral2)

    DEF_NAME("neutral2")
    DELCARE_FRAMES

    SET_DAMAGE(1.2, Vector2::Angled(Helper::DegreesToRadians(-5), 4), 0.3)

    ATTACK_CANCEL(6)
  };

  class SpecialNeutral : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(SpecialNeutral)

    DEF_NAME("specialNeutral")
    DELCARE_FRAMES
  };

}

#endif