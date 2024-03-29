#ifndef __CHARACTER_TEST_ANIMATIONS__
#define __CHARACTER_TEST_ANIMATIONS__

#include "CharacterAnimations.h"

// === ANIMATION DEFINES

#define AIR_DOWN_SHOVEL_LOOP "airDownShovelLoop"

namespace CharacterTestAnimations
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
        Helper::Lerp(1.2f, 6.0f, GetInnerLoopElapsedTime() / MaxInnerLoopDuration()) * BASE_DAMAGE,
        AttackImpulse(Vector2::AngledDegrees(-20),
                      Helper::Lerp(5.5f, 20.0f, GetInnerLoopElapsedTime() / MaxInnerLoopDuration())),
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

    SET_DAMAGE(3.5f * BASE_DAMAGE, AttackImpulse(animator.worldObject.GetShared(), 6), 0.4)
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
    std::weak_ptr<WorldObject> weakParent;

    Projectile(Animator &animator, std::weak_ptr<WorldObject> weakParent)
        : AttackAnimation(animator), weakParent(weakParent) {}
    virtual ~Projectile() {}

    DEF_NAME("projectile")
    DELCARE_FRAMES
    void OnConnectAttack(std::shared_ptr<CharacterController>) override;
    void InternalOnStart() override;

    SET_DAMAGE(1.8, AttackImpulse(animator.worldObject.GetShared(), 2), 0.2)
    FIELD(CycleEndBehavior, EndBehavior, CycleEndBehavior::Loop)
  };

}

#endif