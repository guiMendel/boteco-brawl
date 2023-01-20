#include "GeneralAnimations.h"
#include "ShakeEffectManager.h"
#include "GunParry.h"
#include "ParticleFX.h"
#include "Movement.h"
#include "BoxCollider.h"
#include "CircleCollider.h"
#include "Attack.h"
#include "ObjectRecipes.h"

using namespace std;
using namespace GeneralAnimations;

// Takes the last frame of a sequence, replicates it n - 1 times, and sets all of it's instances duration
void SplitLastFrame(vector<AnimationFrame> &frames, int numberOfInstances, float newDuration);

// === JUMP

vector<AnimationFrame> Jump::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/jump.png",
                               SpritesheetClipInfo(8, 8, 2), 0.1)};

  // Particles to emit on jump
  ParticleEmissionParameters particleEmission;
  particleEmission.angle = {DegreesToRadians(-135), DegreesToRadians(-100)};
  particleEmission.color = {Color::Black(), Color::Gray()};
  particleEmission.frequency = {0.0005, 0.02};
  particleEmission.lifetime = {0.2, 1.0};
  particleEmission.speed = {3, 8};
  particleEmission.gravityModifier = {Vector2::One(), Vector2::One()};

  // Add jump impulse to jump frame
  auto callback = [particleEmission](GameObject &object)
  {
    // Jump
    auto movement = object.RequireComponent<Movement>();
    movement->Jump();

    //  Don't emit if not grounded
    if (movement->IsGrounded() == false)
      return;

    // Get emission position
    auto colliderBox = object.RequireComponent<BoxCollider>()->GetBox();
    auto offset = Vector2(-colliderBox.width / 4, colliderBox.height / 2);

    // Particles
    auto emission = particleEmission;

    if (object.localScale.x < 0)
    {
      offset.x *= -1;
      emission.angle.first = M_PI - emission.angle.first;
      emission.angle.second = M_PI - emission.angle.second;
    }

    ParticleFX::EffectAt(object.GetPosition() + offset, 0.1, 0.1, emission, 3.0);
  };

  frames[1].AddCallback(callback);

  return frames;
}

// === NEUTRAL 1

vector<AnimationFrame> Neutral1::InitializeFrames()
{
  auto frames{Animation::SliceSpritesheet("./assets/sprites/punch.png",
                                          SpritesheetClipInfo(16, 8), 0.1, {4, 0})};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({7.5, 3.5}, 2), Circle({10.5, 3.5}, 2), Circle({13.5, 3.5}, 2)});
  FrameHitbox(frames[2], {Circle({10.5, 3.5}, 2), Circle({13.5, 3.5}, 2)});
  FrameHitbox(frames[3]);

  return frames;
}

// === NEUTRAL 2

vector<AnimationFrame> Neutral2::InitializeFrames()
{
  auto frames{Animation::SliceSpritesheet("./assets/sprites/kick.png",
                                          SpritesheetClipInfo(16, 8), 0.1, {4, 0})};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({8, 4}, 2.5), Circle({12, 4}, 2.5)});
  FrameHitbox(frames[2], {Circle({8, 4}, 2.5), Circle({12, 4}, 2.5)});
  FrameHitbox(frames[3]);

  return frames;
}

// === HORIZONTAL

void Horizontal::InternalOnStart()
{
  if (sequencePhase == SequencePhase::InLoop)
  {
    animator.GetState()->FindObjectOfType<ShakeEffectManager>()->Shake(
        animator.gameObject.GetShared(),
        0,
        {0, 0.15},
        {0.15, 0.08},
        MaxInnerLoopDuration(),
        0);
  }

  InnerLoopAnimation::InternalOnStart();
}

vector<AnimationFrame> Horizontal::InitializeInLoopFrames()
{
  return SliceSpritesheet("./assets/sprites/horizontal.png",
                          SpritesheetClipInfo(24, 8, 1), 0.2);
}

vector<AnimationFrame> Horizontal::InitializePostLoopFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/horizontal.png",
                               SpritesheetClipInfo(24, 8, 3, 1), 0.2)};

  // Stop shake
  auto stopShake = [](GameObject &target)
  {
    target.GetState()->FindObjectOfType<ShakeEffectManager>()->StopShake(
        target.GetShared());
  };
  frames[0].AddCallback(stopShake);

  // Add hitboxes
  FrameHitbox(frames[0], {Circle({16, 3}, 3.5), Circle({21, 3}, 3.5)});
  FrameHitbox(frames[1]);

  // Replicate last frame
  SplitLastFrame(frames, 2, 0.1);

  return frames;
}

// === SPECIAL NEUTRAL

vector<AnimationFrame> SpecialNeutral::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/special.png",
                               SpritesheetClipInfo(16, 8, 1), 0.2, {4, 0})};

  // Add a recovery frame
  frames.push_back(frames[0]);

  // Ready parry
  auto startParry = [](GameObject &object)
  {
    auto parry = object.RequireComponent<GunParry>();

    parry->ready = true;
  };

  auto stopParry = [](GameObject &object)
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
  auto parry = animator.gameObject.RequireComponent<GunParry>();

  parry->ready = false;
}

// === SPECIAL HORIZONTAL

vector<AnimationFrame> SpecialHorizontal::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/special-horizontal.png",
                               SpritesheetClipInfo(16, 8), 0.3, {4, 0})};

  // Add a recovery frame
  SplitLastFrame(frames, 2, 0.15);
  frames[1].SetDuration(0.2);

  // Add shoot frame
  auto shoot = [this](GameObject &target)
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

    auto projectile = animator.GetState()->CreateObject(
        "Projectile",
        ObjectRecipes::Projectile({5 * mirrorFactor, 0}, animator.gameObject.GetShared(), {0, 0}),
        shotPosition);

    projectile->localScale = {mirrorFactor, 1};
  };

  frames[1].AddCallback(shoot);

  return frames;
}

// === RIPOSTE

vector<AnimationFrame> Riposte::InitializeFrames()
{
  auto frames{Animation::SliceSpritesheet("./assets/sprites/special.png",
                                          SpritesheetClipInfo(16, 8, 2, 1), 0.2, {4, 0})};

  // Add hitboxes
  FrameHitbox(frames[0], {Circle({8.5, 4.5}, 7), Circle({14.5, 4.5}, 7)});
  FrameHitbox(frames[1]);

  return frames;
}

// === UP

vector<AnimationFrame> Up::InitializePreLoopFrames()
{
  return Animation::SliceSpritesheet("./assets/sprites/up.png",
                                     SpritesheetClipInfo(12, 16, 2, 0), 0.1, {1, -4});
}

vector<AnimationFrame> Up::InitializeInLoopFrames()
{
  auto frames{Animation::SliceSpritesheet("./assets/sprites/up.png",
                                          SpritesheetClipInfo(12, 16, 2, 2), 0.1, {1, -4})};

  // Add hitboxes
  FrameHitbox(frames[0], {Circle({6, 4}, 3.5), Circle({6, 7}, 3.5)});
  FrameHitbox(frames[1], {Circle({6, 3}, 3.5), Circle({6, 6}, 3.5)});

  return frames;
}

vector<AnimationFrame> Up::InitializePostLoopFrames()
{
  return Animation::SliceSpritesheet("./assets/sprites/up.png",
                                     SpritesheetClipInfo(12, 16, 1, 4), 0.1, {1, -4});
}

// === AIR HORIZONTAL

vector<AnimationFrame> AirHorizontal::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/air-horizontal.png",
                               SpritesheetClipInfo(16, 8), 0.15, {4, 0})};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({7.5, 3.5}, 4)});
  FrameHitbox(frames[2]);

  // Replicate last frame
  SplitLastFrame(frames, 2, 0.15 / 2);

  return frames;
}

// === AIR UP

vector<AnimationFrame> AirUp::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/air-up.png",
                               SpritesheetClipInfo(12, 14), 0.15, {2, -3})};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({5.5, 5.5}, 5)});
  FrameHitbox(frames[2]);

  // Replicate last frame
  SplitLastFrame(frames, 2, 0.15 / 2);

  return frames;
}

// === AIR DOWN

vector<AnimationFrame> AirDown::InitializePreLoopFrames()
{
  return Animation::SliceSpritesheet("./assets/sprites/air-down.png",
                                     SpritesheetClipInfo(8, 20, 1), 0.2, {0, 2});
}

vector<AnimationFrame> AirDown::InitializeInLoopFrames()
{
  auto frames{Animation::SliceSpritesheet("./assets/sprites/air-down.png",
                                          SpritesheetClipInfo(8, 20, 1, 1), 0.1, {0, 2})};

  // Add hitboxes
  FrameHitbox(frames[0], {Circle({4, 14}, 3.5)});

  return frames;
}

vector<AnimationFrame> LandingAttack::InitializeFrames()
{
  auto frames = Animation::SliceSpritesheet("./assets/sprites/air-down.png",
                                            SpritesheetClipInfo(8, 20, 3, 2), 0.2, {0, 2});

  // Halt character
  auto stopVelocity = [](GameObject &target)
  {
    auto body = target.RequireComponent<Rigidbody>();

    body->velocity.x = 0;
  };

  frames[0].AddCallback(stopVelocity);

  return frames;
}

// === PROJECTILE

void Projectile::OnConnectAttack(std::shared_ptr<CharacterController>)
{
  // Play effect
  ParticleEmissionParameters hit;
  hit.color = {Color::Black(), Color::Gray()};
  hit.frequency = {0.001, 0.005};
  hit.lifetime = {0.05, 0.2};
  hit.speed = {1, 3};

  ParticleFX::EffectAt(
      animator.gameObject.GetPosition(),
      0.1,
      0.001,
      hit,
      4);

  // Go away
  animator.gameObject.RequestDestroy();
}

void Projectile::InternalOnStart()
{
  AttackAnimation::InternalOnStart();

  // Don't attack parent
  IF_LOCK(weakParent, parent)
  {
    GetAttack()->Ignore(parent);
  }
}

vector<AnimationFrame> Projectile::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/bullet.png",
                               SpritesheetClipInfo(3, 2), 0.15)};

  // Add hitboxes
  FrameHitbox(frames[0], {Circle({1.0, 0.5}, 1.5)});

  return frames;
}

// === HELPER FUNCTIONS DEFINITIONS

void SplitLastFrame(vector<AnimationFrame> &frames, int numberOfInstances, float newDuration)
{
  Assert(frames.size() > 0, "Can't replicate frames of empty sequence");
  Assert(numberOfInstances > 0, "Can't remove frame");

  auto &lastFrame = frames[frames.size() - 1];

  // Set new duration
  lastFrame.SetDuration(newDuration);

  // Replicate it
  frames.insert(frames.end(), numberOfInstances - 1, lastFrame);
}
