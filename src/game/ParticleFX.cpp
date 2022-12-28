#include "ParticleFX.h"
#include "Game.h"

using namespace std;

ParticleFX::ParticleFX(GameObject &associatedObject)
    : Component(associatedObject) {}

void ParticleFX::EffectAt(Vector2 position, float radius, float duration, ParticleEmissionParameters params, float destroyAfter)
{
  // Find the instance
  auto instance = Game::GetInstance().GetState()->FindObjectOfType<ParticleFX>();

  Assert(instance != nullptr, "Could not find ParticleFX instance");

  instance->PlayEffectAt(position, radius, duration, params, destroyAfter);
}

void ParticleFX::PlayEffectAt(Vector2 position, float radius, float duration, ParticleEmissionParameters params, float destroyAfter)
{
  // Create a temporary child to hold the emitter
  auto emitter = gameObject.CreateChild("FXEmitter", position)
                     ->AddComponent<ParticleEmitter>(RenderLayer::VFX, radius, false, duration);
  emitter->emission = params;

  // Play
  emitter->StartEmission();

  // Add to timer struct
  childrenDestroyTimers[emitter->gameObject.id] = duration + destroyAfter;
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
      auto child = gameObject.GetChild(childId);

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