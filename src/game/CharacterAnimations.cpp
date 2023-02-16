#include "CharacterAnimations.h"
#include "ShakeEffectManager.h"

using namespace std;

void CharacterAnimationHelper::SplitLastFrame(vector<AnimationFrame> &frames, int numberOfInstances, float newDuration)
{
  Assert(frames.size() > 0, "Can't replicate frames of empty sequence");
  Assert(numberOfInstances > 0, "Can't remove frame");

  auto &lastFrame = frames[frames.size() - 1];

  // Set new duration
  lastFrame.SetDuration(newDuration);

  // Replicate it
  frames.insert(frames.end(), numberOfInstances - 1, lastFrame);
}

void CharacterAnimationHelper::ShakeLoop(const InnerLoopAnimation &animation)
{
  if (animation.GetSequencePhase() == InnerLoopAnimation::SequencePhase::InLoop)
  {
    animation.animator.GetScene()->FindComponent<ShakeEffectManager>()->Shake(
        animation.animator.worldObject.GetShared(),
        0,
        {0, 0.15},
        {0.15, 0.08},
        animation.MaxInnerLoopDuration(),
        0);
  }
}

function<void(WorldObject &)> CharacterAnimationHelper::StopShakeCallback()
{
  return [](WorldObject &target)
  {
    target.GetScene()->FindComponent<ShakeEffectManager>()->StopShake(target.GetShared());
  };
}

function<void(WorldObject &)> CharacterAnimationHelper::ResetHitTargetsCallback()
{
  return [](WorldObject &target)
  {
    target.RequireComponentInChildren<Attack>()->ResetTargets();
  };
}

function<void(WorldObject &)> CharacterAnimationHelper::DisplaceCallback(Vector2 displacement)
{
  return [displacement](WorldObject &target)
  {
    // Adjust displacement to facing direction
    target.Translate(target.localScale.x < 0 ? -displacement : displacement);
  };
}

function<void(WorldObject &)> CharacterAnimationHelper::ParticleFXCallback(
    shared_ptr<Sprite> sprite, Vector2 position, float radius, float duration, ParticleEmissionParameters params, float destroyAfter, bool irradiate)
{
  return [position, radius, duration, params, destroyAfter, irradiate, weakSprite = weak_ptr(sprite)](WorldObject &target)
  {
    // Convert virtual pixel position
    auto positionUnits = target.RequireComponent<SpriteRenderer>()->GetVirtualPixelOffset(position, Lock(weakSprite));

    ParticleFX::EffectAt(positionUnits + target.GetPosition(), radius, duration, params, destroyAfter, irradiate);
  };
}