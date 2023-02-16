#ifndef __CHARACTER_KIBA_ANIMATIONS__
#define __CHARACTER_KIBA_ANIMATIONS__

#include "CharacterAnimations.h"

// === ANIMATION DEFINES

#define AIR_BODY_DROP_LOOP "airDownShovelLoop"

namespace CharacterKibaAnimations
{
  class Run : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Run)

    DEF_NAME("run")
    // DEF_FRAMES(SliceSpritesheet("./assets/sprites/kiba/general/run.png", SpritesheetClipInfo(324 / 6, 36), 0.15, {0, -2}))
    DELCARE_FRAMES
    

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Idle : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Idle)

    DEF_NAME("idle")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kiba/general/idle.png", SpritesheetClipInfo(192 / 4, 32), 0.3))

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
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kiba/general/jump.png", SpritesheetClipInfo(480 / 6, 48, 1, 2), 0.1, {0, -8}))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Fall : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Fall)

    DEF_NAME("fall")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kiba/general/jump.png", SpritesheetClipInfo(480 / 6, 48, 1, 4), 0.1, {0, -8}))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Land : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Land)

    DEF_NAME("land")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kiba/general/jump.png", SpritesheetClipInfo(480 / 6, 48, 1, 5), 0.3, {0, -8}))
  };

  class Brake : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Brake)

    DEF_NAME("brake")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kiba/general/dash.png", SpritesheetClipInfo(44, 33, 1, 2), 0.1, {0, -0.5}))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Ouch1 : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Ouch1)

    DEF_NAME("ouch1")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kiba/general/ouch.png", SpritesheetClipInfo(48, 32, 1), 0.2))
  };

  class Ouch2 : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Ouch2)

    DEF_NAME("ouch2")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kiba/general/ouch.png", SpritesheetClipInfo(48, 32, 1, 2), 0.2))
  };

  class Dash : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Dash)

    DEF_NAME("dash")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kiba/general/dash.png", SpritesheetClipInfo(44, 33), 0.1, {0, -0.5}))
  };

  class Spin : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Spin)

    DEF_NAME("spin")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kiba/general/spinning.png", SpritesheetClipInfo(256 / 4, 44), 0.15, {0, -6}))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Crash : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Crash)

    DEF_NAME("crash")
    DELCARE_FRAMES
  };

  class Neutral1 : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(Neutral1)

    DEF_NAME("neutral1")
    DELCARE_FRAMES

    SET_DAMAGE(BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-5), 0.5), 0.4)

    ATTACK_SEQUENCE(3)
  };

  class Neutral2 : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(Neutral2)

    DEF_NAME("neutral2")
    DELCARE_FRAMES

    SET_DAMAGE(1.6f * BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-5), 0.5), 0.5)

    ATTACK_CANCEL(3)
  };

  class Neutral3 : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(Neutral3)

    DEF_NAME("neutral3")
    DELCARE_FRAMES

    SET_DAMAGE(2.3f * BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-5), 5.5), 0.6)
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

    SET_DAMAGE(CHARGE_DAMAGE(2.0f, 9.0f), CHARGE_IMPULSE(-10, 6.0f, 25.0f), 0.7)
  };

  class Up : public InnerLoopAnimation
  {
  public:
    LOOP_CONSTRUCTOR_AND_DESTRUCTOR(Up)

    DEF_FIRST_NAME("up")

    std::vector<AnimationFrame> InitializeInLoopFrames() override;
    std::vector<AnimationFrame> InitializePostLoopFrames() override;
    float MaxInnerLoopDuration() const override { return 2; }
    void InternalOnStart() override;

    SET_DAMAGE(CHARGE_DAMAGE(1.2f, 4.0f), CHARGE_IMPULSE(-89, 3.0f, 12.0f), 0.3)
  };

  class AirHorizontal : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(AirHorizontal)

    DEF_NAME("airHorizontal")
    DELCARE_FRAMES

    SET_DAMAGE(0.9f * BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-35), 1), 0.15)
  };

  class AirUp : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(AirUp)

    DEF_NAME("airUp")
    DELCARE_FRAMES

    SET_DAMAGE(BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-89), 2.5), 0.2)
  };

  class AirDown : public InnerLoopAnimation
  {
  public:
    LOOP_CONSTRUCTOR_AND_DESTRUCTOR(AirDown)

    DEF_FIRST_NAME("airDown")
    std::string Phase2Name() override { return AIR_BODY_DROP_LOOP; }

    std::vector<AnimationFrame> InitializePreLoopFrames() override;
    std::vector<AnimationFrame> InitializeInLoopFrames() override;

    bool QuitLoopOnInputRelease() const override { return false; }
    void InternalOnStop() override;

    SET_DAMAGE(BASE_DAMAGE * 2, AttackImpulse(animator.worldObject.GetShared(), 3), 0.3)
  };

  class SpecialNeutral : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(SpecialNeutral)

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

  class LandingAttack : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(LandingAttack)

    DEF_NAME("landingAttack")
    DELCARE_FRAMES

    SET_DAMAGE(BASE_DAMAGE * 3.5f, AttackImpulse(animator.worldObject.GetShared(), 7), 0.4)

    // Speed at which ground was intercepted
    float landingSpeed;
  };

}

#endif