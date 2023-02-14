#ifndef __CHARACTER_KAFTA_ANIMATIONS__
#define __CHARACTER_KAFTA_ANIMATIONS__

#include "CharacterAnimations.h"

// === ANIMATION DEFINES

#define AIR_DOWN_SHOVEL_LOOP "airDownShovelLoop"

namespace CharacterKaftaAnimations
{
  class Run : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Run)

    DEF_NAME("run")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kafta/general/run.png", SpritesheetClipInfo(48, 48), 0.08, {0, -8}))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Idle : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Idle)

    DEF_NAME("idle")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kafta/general/idle.png", SpritesheetClipInfo(48, 48), 0.5, {0, -8}))

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
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kafta/general/rise.png", SpritesheetClipInfo(48, 48), 0.1, {0, -8}))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Fall : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Fall)

    DEF_NAME("fall")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kafta/general/fall.png", SpritesheetClipInfo(48, 48), 0.1, {0, -8}))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Land : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Land)

    DEF_NAME("land")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kafta/general/land.png", SpritesheetClipInfo(48, 48), 0.15, {0, -8}))
  };

  class Brake : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Brake)

    DEF_NAME("brake")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kafta/general/brake.png", SpritesheetClipInfo(48, 48), 0.1, {0, -8}))

    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

  class Ouch1 : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Ouch1)

    DEF_NAME("ouch1")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kafta/general/ouch1.png", SpritesheetClipInfo(48, 48), 0.2, {0, -8}))
  };

  class Ouch2 : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Ouch2)

    DEF_NAME("ouch2")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kafta/general/ouch2.png", SpritesheetClipInfo(48, 48), 0.2, {0, -8}))
  };

  class Dash : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Dash)

    DEF_NAME("dash")
    DELCARE_FRAMES
  };

  class Spin : public StatefulAnimation
  {
  public:
    CONSTRUCTOR_AND_DESTRUCTOR(Spin)

    DEF_NAME("spin")
    DEF_FRAMES(SliceSpritesheet("./assets/sprites/kafta/general/spinning.png", SpritesheetClipInfo(48, 48), 0.15, {0, -2}))

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

    SET_DAMAGE(BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-5), 0.2), 0.4)

    ATTACK_SEQUENCE(2)
  };

  class Neutral2 : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(Neutral2)

    DEF_NAME("neutral2")
    DELCARE_FRAMES

    SET_DAMAGE(1.2f * BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-5), 0.2), 0.4)
    ATTACK_SEQUENCE(3)
  };

  class Neutral3 : public InnerLoopAnimation
  {
  public:
    LOOP_CONSTRUCTOR_AND_DESTRUCTOR(Neutral3)

    DEF_FIRST_NAME("neutral3")

    std::vector<AnimationFrame> InitializePreLoopFrames() override;
    std::vector<AnimationFrame> InitializeInLoopFrames() override;
    std::vector<AnimationFrame> InitializePostLoopFrames() override;

    SET_DAMAGE(BASE_DAMAGE, AttackImpulse(animator.worldObject.GetShared(), 0.3), 0.2)
    SET_HIT_COOLDOWN(0.2)
  };

  class Horizontal : public InnerLoopAnimation
  {
  public:
    LOOP_CONSTRUCTOR_AND_DESTRUCTOR(Horizontal)

    DEF_FIRST_NAME("horizontal")

    std::vector<AnimationFrame> InitializePreLoopFrames() override;
    std::vector<AnimationFrame> InitializeInLoopFrames() override;
    std::vector<AnimationFrame> InitializePostLoopFrames() override;
    float MaxInnerLoopDuration() const override { return 2; }
    void InternalOnStart() override;

    SET_DAMAGE(
        Helper::Lerp(1.4f, 5.0f, GetInnerLoopElapsedTime() / MaxInnerLoopDuration()) * BASE_DAMAGE,
        AttackImpulse(Vector2::AngledDegrees(-20),
                      Helper::Lerp(2.0f, 8.0f, GetInnerLoopElapsedTime() / MaxInnerLoopDuration())),
        0.6)

    int PostLoopCancelFrame() const override { return 3; }
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

    SET_DAMAGE(
        Helper::Lerp(1.5f, 8.0f, GetInnerLoopElapsedTime() / MaxInnerLoopDuration()) * BASE_DAMAGE,
        AttackImpulse(Vector2::AngledDegrees(-89),
                      Helper::Lerp(1.5f, 6.0f, GetInnerLoopElapsedTime() / MaxInnerLoopDuration())),
        0.6)

    int PostLoopCancelFrame() const override { return 2; }
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

    SET_DAMAGE(1.3f * BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(-89), 3), 0.1)
  };

  class AirDown : public AttackAnimation
  {
  public:
    ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(AirDown)

    DEF_NAME("airDown")
    DELCARE_FRAMES

    SET_DAMAGE(2 * BASE_DAMAGE, AttackImpulse(Vector2::AngledDegrees(89), 3), 0.3)
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

  class Riposte : public RiposteAnimation
  {
  public:
    Riposte(Animator &animator) : RiposteAnimation(animator) {}
    virtual ~Riposte() {}

    DEF_NAME("riposte")
    DELCARE_FRAMES
  };

}

#endif