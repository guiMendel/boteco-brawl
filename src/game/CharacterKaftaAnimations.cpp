#include "CharacterKaftaAnimations.h"
#include "Resources.h"
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
using namespace CharacterKaftaAnimations;
using namespace CharacterAnimationHelper;

// === DASH

vector<AnimationFrame> Dash::InitializeFrames()
{
  auto frames = SliceSpritesheet("./assets/sprites/kafta/general/dash.png", SpritesheetClipInfo(48, 48), 0.3, {0, -8});

  // Use 2 frames
  SplitLastFrame(frames, 2, 0.1);

  // First frame with duration 0.2
  frames[0].SetDuration(0.2);

  return frames;
}

// === JUMP

vector<AnimationFrame> Jump::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kafta/general/jump.png",
                               SpritesheetClipInfo(48, 48), 0.1, {0, -8})};

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
  auto frames = SliceSpritesheet("./assets/sprites/kafta/general/get-up.png", SpritesheetClipInfo(48, 48), 0.117, {0, -8});

  // Add fallen frame
  AnimationFrame fallenFrame{Resources::GetSprite("./assets/sprites/kafta/general/fallen.png"), 0.6};
  fallenFrame.spriteOffset = Vector2{0, -8} / Game::defaultVirtualPixelsPerUnit;

  frames.insert(frames.begin(), fallenFrame);

  return frames;
}

// === NEUTRAL 1

vector<AnimationFrame> Neutral1::InitializeFrames()
{
  auto frames{Animation::SliceSpritesheet("./assets/sprites/kafta/attacks/neutral1.png",
                                          SpritesheetClipInfo(144 / 3, 48), 0.13, {10, -8})};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({37, 30}, 8.5)});
  frames[1].SetDuration(0.1);
  FrameHitbox(frames[2]);

  return frames;
}

// === NEUTRAL 2

vector<AnimationFrame> Neutral2::InitializeFrames()
{
  auto frames{Animation::SliceSpritesheet("./assets/sprites/kafta/attacks/neutral2.png",
                                          SpritesheetClipInfo(48, 48), 0.2, {8, -8})};

  frames[0].SetDuration(0.15);
  frames[1].SetDuration(0.15);
  frames[2].SetDuration(0.1);

  SplitLastFrame(frames, 2, 0.15);

  // Add hitboxes
  FrameHitbox(frames[2], {Circle({31, 32}, 15.5)});
  FrameHitbox(frames[3]);

  return frames;
}

// === NEUTRAL 3

vector<AnimationFrame> Neutral3::InitializePreLoopFrames()
{
  return Animation::SliceSpritesheet("./assets/sprites/kafta/attacks/draw-sword.png",
                                     SpritesheetClipInfo(48, 48, 1), 0.2, {0, -8});
}

vector<AnimationFrame> Neutral3::InitializeInLoopFrames()
{
  auto frames{Animation::SliceSpritesheet("./assets/sprites/kafta/attacks/neutral3.png",
                                          SpritesheetClipInfo(64, 48), 0.2, {0, -8})};

  // Add hitboxes
  FrameHitbox(frames[0], {Circle({16, 32}, 16.5), Circle({47, 32}, 16.5)});

  return frames;
}

vector<AnimationFrame> Neutral3::InitializePostLoopFrames()
{
  return Animation::SliceSpritesheet("./assets/sprites/kafta/attacks/draw-sword.png",
                                     SpritesheetClipInfo(48, 48, 1, 1), 0.4, {0, -8});
}

// === HORIZONTAL

void Horizontal::InternalOnStart()
{
  // if (sequencePhase == SequencePhase::InLoop)
  // {
  //   animator.GetScene()->FindComponent<ShakeEffectManager>()->Shake(
  //       animator.worldObject.GetShared(),
  //       0,
  //       {0, 0.15},
  //       {0.15, 0.08},
  //       MaxInnerLoopDuration(),
  //       0);
  // }

  ShakeLoop(*this);

  InnerLoopAnimation::InternalOnStart();
}

vector<AnimationFrame> Horizontal::InitializePreLoopFrames()
{
  return Animation::SliceSpritesheet("./assets/sprites/kafta/attacks/draw-sword.png",
                                     SpritesheetClipInfo(48, 48, 1), 0.15, {0, -8});
}

vector<AnimationFrame> Horizontal::InitializeInLoopFrames()
{
  return SliceSpritesheet("./assets/sprites/kafta/attacks/horizontal.png",
                          SpritesheetClipInfo(108, 48, 1), 0.3, {0, -8});
}

vector<AnimationFrame> Horizontal::InitializePostLoopFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kafta/attacks/horizontal.png",
                               SpritesheetClipInfo(108, 48, 2, 1), 0.25, {0, -8})};

  // Stop shake
  frames[0].AddCallback(StopShakeCallback());
  // auto stopShake = [](WorldObject &target)
  // {
  //   target.GetScene()->FindComponent<ShakeEffectManager>()->StopShake(
  //       target.GetShared());
  // };

  // Add recovery
  auto recoveryFrames{SliceSpritesheet("./assets/sprites/kafta/attacks/draw-sword.png",
                                       SpritesheetClipInfo(48, 48, 1), 0.4, {0, -8})};

  frames.push_back(recoveryFrames[0]);

  SplitLastFrame(frames, 2, 0.4);
  frames[3].SetDuration(0.2);

  // Add hitboxes
  FrameHitbox(frames[0], {Circle({53, 31}, 16.5), Circle({72, 30}, 17.5)});
  FrameHitbox(frames[1], {Circle({52.5, 23.5}, 25), Circle({70.5, 28.5}, 20)});
  FrameHitbox(frames[2]);

  // Add displacement
  frames[0].AddCallback(DisplaceCallback({1.2, 0}));
  frames[1].AddCallback(DisplaceCallback({1.2, 0}));

  // Enable second hit connection
  frames[1].AddCallback(ResetHitTargetsCallback());

  return frames;
}

// === SPECIAL NEUTRAL

vector<AnimationFrame> SpecialNeutral::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kafta/attacks/special.png",
                               SpritesheetClipInfo(16, 8, 1), 0.2, {4, 0})};

  // Add a recovery frame
  frames.push_back(frames[0]);

  // Ready parry
  auto startParry = [](WorldObject &object)
  {
    auto parry = object.RequireComponent<GunParry>();

    parry->ready = true;
  };

  auto stopParry = [](WorldObject &object)
  {
    auto parry = object.RequireComponent<GunParry>();

    parry->ready = false;
  };

  frames[0].AddCallback(startParry);
  frames[1].AddCallback(stopParry);

  return frames;
}

void SpecialNeutral::InternalOnStop()
{
  auto parry = animator.worldObject.RequireComponent<GunParry>();

  parry->ready = false;
}

// === SPECIAL HORIZONTAL

vector<AnimationFrame> SpecialHorizontal::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kafta/attacks/special-horizontal.png",
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

// === RIPOSTE

vector<AnimationFrame> Riposte::InitializeFrames()
{
  auto frames{Animation::SliceSpritesheet("./assets/sprites/kafta/attacks/special.png",
                                          SpritesheetClipInfo(16, 8, 2, 1), 0.2, {4, 0})};

  // Add hitboxes
  FrameHitbox(frames[0], {Circle({8.5, 4.5}, 7), Circle({14.5, 4.5}, 7)});
  FrameHitbox(frames[1]);

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
  return SliceSpritesheet("./assets/sprites/kafta/attacks/up.png",
                          SpritesheetClipInfo(48, 64, 1), 0.15, {0, -16});
}

vector<AnimationFrame> Up::InitializePostLoopFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kafta/attacks/up.png",
                               SpritesheetClipInfo(48, 64, 2, 1), 0.2, {0, -16})};

  frames[0].AddCallback(StopShakeCallback());

  // Replicate last frame
  SplitLastFrame(frames, 2, 0.15);

  // Add hitboxes
  FrameHitbox(frames[0], {Circle({23.5, 7.5}, 8), Circle({23.5, 17.5}, 8), Circle({23.5, 27.5}, 8), Circle({23.5, 37.5}, 8)});
  FrameHitbox(frames[1]);

  return frames;
}

// === AIR HORIZONTAL

vector<AnimationFrame> AirHorizontal::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kafta/attacks/air-horizontal.png",
                               SpritesheetClipInfo(96, 64), 0.2, {13, -10})};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({48.5, 37.5}, 23), Circle({67, 36}, 18.5)});
  FrameHitbox(frames[2]);

  frames[1].SetDuration(0.12);

  // Replicate last frame
  SplitLastFrame(frames, 2, 0.15);

  return frames;
}

// === AIR UP

vector<AnimationFrame> AirUp::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kafta/attacks/air-up.png",
                               SpritesheetClipInfo(64, 80), 0.2, {0, -19})};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({31, 30}, 28.5)});
  FrameHitbox(frames[2]);

  frames[1].SetDuration(0.12);

  return frames;
}

// === AIR DOWN

vector<AnimationFrame> AirDown::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/kafta/attacks/air-down.png",
                               SpritesheetClipInfo(48, 96), 0.2, {0, 35})};

  // Add hitboxes
  FrameHitbox(frames[1],
              {Circle({25.5, 24.5}, 9),
               Circle({25.5, 36.5}, 9),
               Circle({25.5, 48.5}, 9),
               Circle({25.5, 60.5}, 9),
               Circle({25.5, 72.5}, 9)});
  FrameHitbox(frames[2]);

  frames[1].SetDuration(0.12);

  return frames;
}
