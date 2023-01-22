#ifndef __ANIMATION_RECIPES__
#define __ANIMATION_RECIPES__

#include <memory>
#include "NewAnimationTypes.h"
#include "Animator.h"

// === ANIMATION DEFINES

#define AIR_DOWN_SHOVEL_LOOP "airDownShovelLoop"

// === HELPERS

#define CONSTRUCTOR_AND_DESTRUCTOR_WITH_PARENT(ClassName, Parent) \
  ClassName(Animator &animator) : Parent(animator)                \
  {                                                               \
  }                                                               \
  virtual ~ClassName()                                            \
  {                                                               \
  }

#define CONSTRUCTOR_AND_DESTRUCTOR(ClassName) CONSTRUCTOR_AND_DESTRUCTOR_WITH_PARENT(ClassName, StatefulAnimation)
#define ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(ClassName) CONSTRUCTOR_AND_DESTRUCTOR_WITH_PARENT(ClassName, AttackAnimation)
#define LOOP_CONSTRUCTOR_AND_DESTRUCTOR(ClassName) CONSTRUCTOR_AND_DESTRUCTOR_WITH_PARENT(ClassName, InnerLoopAnimation)

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

#define DEF_NAME(name)        \
  std::string Name() override \
  {                           \
    return name;              \
  }

#define DEF_FIRST_NAME(name)        \
  std::string Phase1Name() override \
  {                                 \
    return name;                    \
  }

#define DECLARE(Type, Name) Type &Name() override;

#define SET_DAMAGE(damage, impulse, stunTime)           \
  DamageParameters GetAttackProperties() const override \
  {                                                     \
    return {damage, impulse, stunTime};                 \
  }

#define SET_HIT_COOLDOWN(cooldown)      \
  float GetHitCooldown() const override \
  {                                     \
    return cooldown;                    \
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

#define BASE_DAMAGE animator.gameObject.RequireComponent<Character>()->GetBaseDamage()

class Animator;

namespace GeneralAnimations
{
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
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/jump.png", SpritesheetClipInfo(8, 8, 1, 4), 0.15))
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
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/ouch.png", SpritesheetClipInfo(8, 8, 1), 0.2))
  };

  class Ouch2 : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Ouch2)

    DEF_NAME("ouch2")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/ouch.png", SpritesheetClipInfo(8, 8, 1, 1), 0.2))
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

    SET_DAMAGE(BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-5), 0.5), 0.2)

    ATTACK_SEQUENCE(3)
    ATTACK_CANCEL(4)
  };

  class Neutral2 : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(Neutral2)

    DEF_NAME("neutral2")
    DELCARE_FRAMES

    SET_DAMAGE(1.2f * BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-5), 4), 0.3)

    ATTACK_CANCEL(5)
  };

  class Horizontal : public InnerLoopAnimation
  {
  public:
    LOOP_CONSTRUCTOR_AND_DESTRUCTOR(Horizontal)

    DEF_FIRST_NAME("horizontal")

    std::vector<AnimationFrame> InitializeInLoopFrames() override;
    std::vector<AnimationFrame> InitializePostLoopFrames() override;
    float MaxInnerLoopDuration() const override { return 2; }
    void InternalOnStart() override;

    SET_DAMAGE(
        Helper::Lerp(1.2f, 3.0f, GetInnerLoopElapsedTime() / MaxInnerLoopDuration()) * BASE_DAMAGE,
        AttackImpulse(Vector2::AngledDegrees(-20),
                      Helper::Lerp(5.5f, 10.0f, GetInnerLoopElapsedTime() / MaxInnerLoopDuration())),
        0.6)

    int PostLoopCancelFrame() const override { return 3; }
  };

  class Up : public InnerLoopAnimation
  {
  public:
    LOOP_CONSTRUCTOR_AND_DESTRUCTOR(Up)

    DEF_FIRST_NAME("up")

    std::vector<AnimationFrame> InitializePreLoopFrames() override;
    std::vector<AnimationFrame> InitializeInLoopFrames() override;
    std::vector<AnimationFrame> InitializePostLoopFrames() override;

    SET_DAMAGE(2 * BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-90), 1.5), 0.2)
    SET_HIT_COOLDOWN(0.2)
  };

  class AirHorizontal : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(AirHorizontal)

    DEF_NAME("airHorizontal")
    DELCARE_FRAMES

    SET_DAMAGE(1.5f * BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-5), 3), 0.3)

    ATTACK_CANCEL(3)
  };

  class AirUp : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(AirUp)

    DEF_NAME("airUp")
    DELCARE_FRAMES

    SET_DAMAGE(0.5f * BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-90), 9), 0.1)

    ATTACK_CANCEL(3)
  };

  class AirDown : public InnerLoopAnimation
  {
  public:
    LOOP_CONSTRUCTOR_AND_DESTRUCTOR(AirDown)

    DEF_FIRST_NAME("airDown")
    std::string Phase2Name() override { return AIR_DOWN_SHOVEL_LOOP; }

    std::vector<AnimationFrame> InitializePreLoopFrames() override;
    std::vector<AnimationFrame> InitializeInLoopFrames() override;

    bool QuitLoopOnInputRelease() const override { return false; }

    SET_DAMAGE(4.5f * BASE_DAMAGE, AttackImpulse(animator.gameObject.GetShared(), 6), 0.4)
  };

  class SpecialNeutral : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(SpecialNeutral)

    void InternalOnStop() override;

    DEF_NAME("specialNeutral")
    DELCARE_FRAMES
  };

  class SpecialHorizontal : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(SpecialHorizontal)

    DEF_NAME("specialHorizontal")
    DELCARE_FRAMES
    ATTACK_CANCEL(3)
  };

  class Riposte : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(Riposte)

    DEF_NAME("riposte")
    DELCARE_FRAMES

    DamageParameters GetAttackProperties() const override { return damage; }

    // Damage properties vary according to the parry
    DamageParameters damage;
  };

  class LandingAttack : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(LandingAttack)

    DEF_NAME("landingAttack")
    DELCARE_FRAMES

    // Speed at which ground was intercepted
    float landingSpeed;
  };

  class Spin : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Spin)

    DEF_NAME("spin")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/spinning.png", SpritesheetClipInfo(10, 10), 0.15))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Crash : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Crash)

    DEF_NAME("crash")
    DELCARE_FRAMES
  };

  class Projectile : public AttackAnimation
  {
  public:
    std::weak_ptr<GameObject> weakParent;

    Projectile(Animator &animator, std::weak_ptr<GameObject> weakParent)
        : AttackAnimation(animator), weakParent(weakParent) {}
    ~Projectile() {}

    DEF_NAME("projectile")
    DELCARE_FRAMES
    void OnConnectAttack(std::shared_ptr<CharacterController>) override;
    void InternalOnStart() override;

    SET_DAMAGE(1.8, AttackImpulse(animator.gameObject.GetShared(), 2), 0.2)
    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

}

#endif