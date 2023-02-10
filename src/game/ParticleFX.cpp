#include "ParticleFX.h"
#include "Game.h"

using namespace std;

ParticleFX::ParticleFX(GameObject &associatedObject)
    : WorldComponent(associatedObject) {}

void ParticleFX::EffectAt(Vector2 position, float radius, float duration, ParticleEmissionParameters params, float destroyAfter)
{
  // Find the instance
  auto instance = Game::GetInstance().GetScene()->FindComponent<ParticleFX>();

  Assert(instance != nullptr, "Could not find ParticleFX instance");

  instance->PlayEffectAt(position, radius, duration, params, destroyAfter);
}

void ParticleFX::PlayEffectAt(Vector2 position, float radius, float duration, ParticleEmissionParameters params, float destroyAfter)
{
  // Create a temporary child to hold the emitter
  auto emitter = worldObject.CreateChild("FXEmitter", position)
                     ->AddComponent<ParticleEmitter>(RenderLayer::VFX,
                     make_unique<Circle>(radius),
                     false,
                     duration);
  emitter->emission = params;

  // Play
  emitter->StartEmission();

  // Add to timer struct
  childrenDestroyTimers[emitter->worldObject.id] = duration + destroyAfter;
}

void ParticleFX::Update(float deltaTime)
{
  // Count down timers
  auto timerIterator = childrenDestroyTimers.begin();
  while (timerIterator != childrenDestroyTimers.end())
  {
    auto &[childId, timer] = *timerIterator;

    // Discount time
    if ((timer -= deltaTime) <= 0)
    {
      // Get child
      auto child = worldObject.GetChild(childId);

      Assert(child != nullptr, "Unexpectedly failed to get ParticleFX child by id");

      // Stop emission
      child->RequireComponent<ParticleEmitter>()->Stop();

      // Destroy child
      child->RequestDestroy();

      // Remove entry
      timerIterator = childrenDestroyTimers.erase(timerIterator);
    }
    else
      timerIterator++;
  }
}

Particle::behavior_callback ParticleBehavior::Accelerate(Vector2 acceleration)
{
  return [acceleration](Particle &particle, float deltaTime)
  {
    particle.velocity += acceleration * deltaTime;
  };
}

Particle::behavior_callback ParticleBehavior::Accelerate(Vector2 acceleration, Vector2 maxVelocity)
{
  return [acceleration, maxVelocity](Particle &particle, float deltaTime)
  {
    particle.velocity += acceleration * deltaTime;

    // Apply max values
    if (acceleration.x >= 0)
      particle.velocity.x = min(particle.velocity.x, maxVelocity.x);
    else
      particle.velocity.x = max(particle.velocity.x, maxVelocity.x);

    if (acceleration.y >= 0)
      particle.velocity.y = min(particle.velocity.y, maxVelocity.y);
    else
      particle.velocity.y = max(particle.velocity.y, maxVelocity.y);
  };
}
