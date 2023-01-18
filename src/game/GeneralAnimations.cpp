#include "GeneralAnimations.h"
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

vector<AnimationFrame> Horizontal::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/horizontal.png",
                               SpritesheetClipInfo(24, 8), 0.2)};

  // Add hitboxes
  FrameHitbox(frames[1], {Circle({16, 3}, 3.5), Circle({21, 3}, 3.5)});
  FrameHitbox(frames[2]);

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
  LOCK(weakAnimator, animator);

  auto parry = animator->gameObject.RequireComponent<GunParry>();

  parry->ready = false;
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
