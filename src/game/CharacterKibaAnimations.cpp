#include "CharacterKibaAnimations.h"
#include "CharacterVFX.h"
#include "ShakeEffectManager.h"
#include "GunParry.h"
#include "ParticleFX.h"
#include "Movement.h"
#include "BoxCollider.h"
#include "CircleCollider.h"
#include "Attack.h"
#include "ObjectRecipes.h"

using namespace std;
using namespace CharacterKibaAnimations;
using namespace CharacterAnimationHelper;

// === JUMP

vector<AnimationFrame> Jump::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kiba/general/jump.png",
                               SpritesheetClipInfo(480 / 6, 48, 2), 0.1, {0, -8})};

  // Add jump impulse to jump frame
  auto callback = [](WorldObject &object)
  {
    // Jump
    auto movement = object.RequireComponent<Movement>();
    movement->Jump();

    //  Don't emit if not grounded
    if (movement->IsGrounded() == false)
      return;

    // Get emission position
    auto colliderBox = object.RequireComponent<BoxCollider>()->GetBox();

    object.RequireComponent<CharacterVFX>()->PlayDust(
        Vector2(-colliderBox.width / 4, colliderBox.height / 2),
        {DegreesToRadians(-135), DegreesToRadians(-100)});
  };

  frames[1].AddCallback(callback);

  return frames;
}

// === CRASH

vector<AnimationFrame> Crash::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kiba/general/get-up.png",
                               SpritesheetClipInfo(56, 32), 0.15)};

  frames[0].SetDuration(0.6);

  return frames;
}

// === NEUTRAL 1

vector<AnimationFrame> Neutral1::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kiba/attacks/neutral.png",
                               SpritesheetClipInfo(720 / 9, 32, 3), 0.3, {17, 0})};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({122.5 - 80, 14.5}, 13)});
  FrameHitbox(frames[2]);

  frames[1].SetDuration(0.15);

  SplitLastFrame(frames, 2, 0.2);
  frames[2].SetDuration(0.1);

  return frames;
}

// === NEUTRAL 2

vector<AnimationFrame> Neutral2::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kiba/attacks/neutral.png",
                               SpritesheetClipInfo(720 / 9, 32, 3, 3), 0.3, {17, 0})};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({368 - 4 * 80, 14}, 15.5)});
  FrameHitbox(frames[2]);

  frames[1].SetDuration(0.2);

  SplitLastFrame(frames, 2, 0.2);
  frames[2].SetDuration(0.1);

  return frames;
}

// === NEUTRAL 3

vector<AnimationFrame> Neutral3::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kiba/attacks/neutral.png",
                               SpritesheetClipInfo(720 / 9, 32, 3, 6), 0.3, {17, 0})};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({609 - 7 * 80, 15}, 15.5)});
  FrameHitbox(frames[2]);

  frames[1].SetDuration(0.2);
  frames[2].SetDuration(0.4);

  return frames;
}

// === HORIZONTAL

void Horizontal::InternalOnStart()
{
  ShakeLoop(*this);

  InnerLoopAnimation::InternalOnStart();
}

vector<AnimationFrame> Horizontal::InitializeInLoopFrames()
{
  return SliceSpritesheet("./assets/sprites/kiba/attacks/horizontal.png",
                          SpritesheetClipInfo(240 / 3, 47, 1), 0.4, {18, -3});
}

vector<AnimationFrame> Horizontal::InitializePostLoopFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kiba/attacks/horizontal.png",
                               SpritesheetClipInfo(240 / 3, 47, 2, 1), 0.5, {18, -3})};

  // Stop shake
  frames[0].AddCallback(StopShakeCallback());

  // Add hitboxes
  FrameHitbox(frames[0], {Circle({152.5 - 80, 23.5}, 20), Circle({132 - 80, 24}, 17.5)});
  FrameHitbox(frames[1]);

  // Step forward
  frames[0].AddCallback(DisplaceCallback({1.3, 0}));

  // Replicate last frame
  SplitLastFrame(frames, 1, 0.25);

  return frames;
}

// === SPECIAL NEUTRAL

vector<AnimationFrame> SpecialNeutral::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kiba/attacks/special.png",
                               SpritesheetClipInfo(80, 48), 0.3, {4, 0})};

  frames[1].SetDuration(0.15);
  frames[2].SetDuration(0.15);

  // TODO: IMPLEMENTAR MECANICA

  return frames;
}

// === SPECIAL HORIZONTAL

vector<AnimationFrame> SpecialHorizontal::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kiba/attacks/special-horizontal.png",
                               SpritesheetClipInfo(16, 8), 0.3, {4, 0})};

  // Add a recovery frame
  SplitLastFrame(frames, 2, 0.15);
  frames[1].SetDuration(0.2);

  // Add shoot frame
  auto shoot = [this](WorldObject &target)
  {
    float mirrorFactor = GetSign(target.GetScale().x);

    // Get shoot position
    Vector2 shotPosition = GlobalVirtualPixelPosition({10, 3});

    // Add smoke
    ParticleEmissionParameters smoke;
    smoke.angle = {DegreesToRadians(-35), DegreesToRadians(35)};
    smoke.color = {Color(90, 90, 90), Color(200, 200, 200)};
    smoke.frequency = {0.0005, 0.001};
    smoke.gravityModifier = {Vector2::Up(0.5), Vector2::Zero()};
    smoke.lifetime = {0.1, 0.7};
    smoke.speed = {0.5 * mirrorFactor, 5 * mirrorFactor};
    smoke.behavior = ParticleBehavior::Accelerate({-mirrorFactor, 0}, {0, numeric_limits<float>::max()});

    ParticleFX::EffectAt(shotPosition, 0.01, 0.01, smoke, 1);

    // Add sparks
    ParticleEmissionParameters sparks;
    sparks.angle = {DegreesToRadians(-35), DegreesToRadians(35)};
    sparks.color = {Color::Yellow(), Color::ClampValid(Color::Yellow() * 1.5)};
    sparks.frequency = {0.0008, 0.003};
    sparks.lifetime = {0.05, 0.3};
    sparks.speed = {2 * mirrorFactor, 8 * mirrorFactor};

    ParticleFX::EffectAt(shotPosition, 0.01, 0.01, sparks, 1);

    auto projectile = animator.GetScene()->Instantiate(
        "Projectile",
        ObjectRecipes::Projectile({8 * mirrorFactor, 0}, animator.worldObject.GetShared(), {0, 0}),
        shotPosition);

    projectile->localScale = {mirrorFactor, 1};
  };

  frames[1].AddCallback(shoot);

  return frames;
}

// === UP

void Up::InternalOnStart()
{
  ShakeLoop(*this);

  InnerLoopAnimation::InternalOnStart();
}

vector<AnimationFrame> Up::InitializeInLoopFrames()
{
  return SliceSpritesheet("./assets/sprites/kiba/attacks/up.png",
                          SpritesheetClipInfo(192 / 3, 64, 1), 0.2, {2, -16});
}

vector<AnimationFrame> Up::InitializePostLoopFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kiba/attacks/up.png",
                               SpritesheetClipInfo(192 / 3, 64, 2, 1), 0.2, {2, -16})};

  // Stop shake
  frames[0].AddCallback(StopShakeCallback());

  // Add hitboxes
  FrameHitbox(frames[0], {Circle({95.5 - 192 / 3, 9.5}, 20)});
  FrameHitbox(frames[1]);

  return frames;
}

// === AIR HORIZONTAL

vector<AnimationFrame> AirHorizontal::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kiba/attacks/air-horizontal.png",
                               SpritesheetClipInfo(80, 40), 0.2, {0 , -4})};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({132.5 - 80, 19.5}, 13)});
  FrameHitbox(frames[2]);

  frames[1].SetDuration(0.15);

  return frames;
}

// === AIR UP

vector<AnimationFrame> AirUp::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kiba/attacks/air-up.png",
                               SpritesheetClipInfo(80, 64), 0.25, {0, -16})};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({120 - 80, 20}, 17.5)});
  FrameHitbox(frames[2]);

  frames[1].SetDuration(0.2);

  return frames;
}

// === AIR DOWN

vector<AnimationFrame> AirDown::InitializePreLoopFrames()
{
  return SliceSpritesheet("./assets/sprites/kiba/attacks/air-down.png",
                          SpritesheetClipInfo(8, 20, 1), 0.2, {0, 2});
}

vector<AnimationFrame> AirDown::InitializeInLoopFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kiba/attacks/air-down.png",
                               SpritesheetClipInfo(8, 20, 1, 1), 0.1, {0, 2})};

  // Add hitboxes
  FrameHitbox(frames[0], {Circle({4, 14}, 3.5)});

  return frames;
}

vector<AnimationFrame> LandingAttack::InitializeFrames()
{
  auto frames = SliceSpritesheet("./assets/sprites/kiba/attacks/air-down.png",
                                 SpritesheetClipInfo(8, 20, 3, 2), 0.2, {0, 2});

  // Halt character
  auto stopVelocity = [](WorldObject &target)
  {
    auto body = target.RequireComponent<Rigidbody>();

    body->velocity.x = 0;
  };

  frames[0].AddCallback(stopVelocity);

  return frames;
}
