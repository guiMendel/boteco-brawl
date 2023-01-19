#include "ShakeEffectManager.h"
#include "SpriteRenderer.h"

using namespace std;

ShakeEffectManager::ShakeEffectManager(GameObject &associatedObject) : Component(associatedObject) {}

void ShakeEffectManager::Shake(shared_ptr<GameObject> target,
                               float angle,
                               pair<float, float> displacementEvolution,
                               pair<float, float> revolutionTimeEvolution,
                               float effectDuration,
                               float stopDuration)
{
  // Stop any current effects for this object
  StopShake(target, 0);

  // Create effect
  activeShakes[target->id] = make_unique<ShakeEffect>(
      target, *this, angle, displacementEvolution, revolutionTimeEvolution, effectDuration, stopDuration);
}

void ShakeEffectManager::StopShake(shared_ptr<GameObject> target, float overrideStopDuration)
{
  if (activeShakes.count(target->id) == 0)
    return;

  // Get the effect
  auto &effect = activeShakes[target->id];

  // Apply override
  if (overrideStopDuration > 0)
    effect->stopDuration = overrideStopDuration;

  // Stop it
  effect->Stop();
}

void ShakeEffectManager::EraseShake(int targetId) { activeShakes.erase(targetId); }

void ShakeEffectManager::PhysicsUpdate(float deltaTime)
{
  // Update each effect
  for (auto &[targetId, effect] : activeShakes)
    effect->Update(deltaTime);
}

ShakeEffect::ShakeEffect(shared_ptr<GameObject> target,
                         ShakeEffectManager &manager,
                         float angle,
                         pair<float, float> displacementEvolution,
                         pair<float, float> revolutionTimeEvolution,
                         float effectDuration,
                         float stopDuration)
    : manager(manager),
      weakRenderer(target->RequireComponent<SpriteRenderer>()),
      angle(angle),
      timeToLive(effectDuration),
      maxDisplacement(displacementEvolution.first),
      revolutionTime(revolutionTimeEvolution.first),
      maxDisplacementChange((displacementEvolution.second - displacementEvolution.first) / effectDuration),
      revolutionTimeChange((revolutionTimeEvolution.second - revolutionTimeEvolution.first) / effectDuration),
      stopDuration(stopDuration)
{
  LOCK(weakRenderer, renderer);

  // Store original offset
  originalSpriteOffset = renderer->GetOffset();

  // Keep it updated
  renderer->OnSetOffset.AddListener("shake-effect-" + to_string(target->id), [this](Vector2 newOffset)
                                    { if (ignoreNewOffsetEvent == false) originalSpriteOffset = newOffset; });
}

ShakeEffect::~ShakeEffect()
{
  cout << "Effect collected" << endl;
  
  LOCK(weakRenderer, renderer);

  // Clean up listener
  renderer->OnSetOffset.RemoveListener("shake-effect-" + to_string(renderer->gameObject.id));

  // Reset any modifications
  Reset();
}

void ShakeEffect::Reset()
{
  // Restore original offset
  SetOffset(originalSpriteOffset);
}

void ShakeEffect::SetOffset(Vector2 value)
{
  LOCK(weakRenderer, renderer);

  // Set ignore flag
  ignoreNewOffsetEvent = true;

  // Set new value
  renderer->SetOffset(value);

  // Disable flag
  ignoreNewOffsetEvent = false;
}

void ShakeEffect::Stop()
{
  LOCK(weakRenderer, renderer);

  // If already in stop time (or there is no stop time), erase self
  if (inStopTime || stopDuration <= 0)
  {
    manager.EraseShake(renderer->gameObject.id);
    return;
  }

  // Set stop time
  inStopTime = true;

  // Set time of stop duration
  timeToLive = stopDuration;

  // Set speeds
  maxDisplacementChange = -maxDisplacement / stopDuration;
  revolutionTimeChange = -revolutionTime / stopDuration;
}

void ShakeEffect::Update(float deltaTime)
{
  // If no time for revolution, nothing to do
  if (revolutionTime == 0)
  {
    Reset();
    return;
  }

  // Apply param speeds
  maxDisplacement += maxDisplacementChange * deltaTime;
  revolutionTime += revolutionTimeChange * deltaTime;

  // Find out which speed to apply to displacement
  float speed;

  // Check if already passed max displacement
  if (abs(displacement) >= maxDisplacement)
  {
    // Ensure it will reduce if it's positive, and reduce if negative
    increasingDisplacement = displacement < 0;

    // Calculate speed over actual distance to displace back
    speed = (3 * maxDisplacement + abs(displacement)) / revolutionTime;
  }

  // Otherwise, calculate speed off of params
  else
    speed = 4 * maxDisplacement / revolutionTime;

  // Calculate new displacement
  displacement += speed * (increasingDisplacement ? 1 : -1) * deltaTime;

  // Apply it
  SetOffset(originalSpriteOffset + Vector2::Angled(angle, displacement));

  // Count time
  if ((timeToLive -= deltaTime) <= 0)
    Stop();
}
