#include "ShakeEffectManager.h"
#include "SpriteRenderer.h"

using namespace std;

ShakeEffectManager::ShakeEffectManager(WorldObject &associatedObject) : Component(associatedObject) {}

void ShakeEffectManager::OnBeforeDestroy()
{
  // Stop all effects and collect them
  auto shakeEntryIterator = activeShakes.begin();
  while (shakeEntryIterator != activeShakes.end())
  {
    StopShake(shakeEntryIterator->first, 0);

    shakeEntryIterator = activeShakes.erase(shakeEntryIterator);
  }
}

void ShakeEffectManager::Shake(shared_ptr<WorldObject> target,
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

void ShakeEffectManager::StopShake(shared_ptr<WorldObject> target, float overrideStopDuration)
{
  StopShake(target->id, overrideStopDuration);
}

void ShakeEffectManager::StopShake(int targetId, float overrideStopDuration)
{
  if (activeShakes.count(targetId) == 0)
    return;

  // Get the effect
  auto &effect = activeShakes[targetId];

  // Apply override
  if (overrideStopDuration > 0)
    effect->stopDuration = overrideStopDuration;

  // Stop it
  effect->Stop();
}

void ShakeEffectManager::Update(float deltaTime)
{
  // Update each effect
  auto shakeEntryIterator = activeShakes.begin();
  while (shakeEntryIterator != activeShakes.end())
  {
    if (shakeEntryIterator->second->destroyRequested)
      shakeEntryIterator = activeShakes.erase(shakeEntryIterator);
    else
      (shakeEntryIterator++)->second->Update(deltaTime);
  }
}

ShakeEffect::ShakeEffect(shared_ptr<WorldObject> target,
                         ShakeEffectManager &manager,
                         float angle,
                         pair<float, float> displacementEvolution,
                         pair<float, float> revolutionTimeEvolution,
                         float effectDuration,
                         float stopDuration)
    : manager(manager),
      weakRenderer(target->RequireComponent<SpriteRenderer>()),
      angle(angle),
      duration(effectDuration),
      timeToLive(effectDuration),
      displacementEvolution(displacementEvolution),
      revolutionTimeEvolution(revolutionTimeEvolution),
      maxDisplacement(displacementEvolution.first),
      revolutionTime(revolutionTimeEvolution.first),
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

  IF_LOCK(weakRenderer, renderer)
  {
    // Clean up listener
    renderer->OnSetOffset.RemoveListener("shake-effect-" + to_string(renderer->worldObject.id));
  }

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
  IF_NOT_LOCK(weakRenderer, renderer)
  {
    return;
  }

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

  // Add stop time
  timeToLive += stopDuration;

  // If already in stop time (or there is no stop time) or even by adding stop time it's still up, erase self
  if (inStopTime || stopDuration <= 0 || timeToLive <= 0)
  {
    destroyRequested = true;
    return;
  }

  // Set stop time
  inStopTime = true;
  displacementEvolution = {maxDisplacement, 0};
  revolutionTimeEvolution = {revolutionTime, 0.0001};

  duration = stopDuration;
}

void ShakeEffect::Update(float deltaTime)
{
  // Count time
  if ((timeToLive -= deltaTime) <= 0)
  {
    Stop();
    return;
  }

  // Evolve params
  maxDisplacement = Lerp(displacementEvolution, (duration - timeToLive) / duration);
  revolutionTime = Lerp(revolutionTimeEvolution, (duration - timeToLive) / duration);

  // If no time for revolution, nothing to do
  if (revolutionTime == 0 || maxDisplacement == 0)
  {
    Reset();
    return;
  }

  // Find out which speed to apply to displacement
  float modification;

  // Check if already passed max displacement
  if (abs(displacement) >= maxDisplacement)
  {
    // Ensure it will reduce if it's positive, and raise if negative
    if (displacement < 0)
    {
      displacementDirection = 1;
      displacement = -maxDisplacement;
    }
    else
    {
      displacementDirection = -1;
      displacement = maxDisplacement;
    }
  }

  // Get a priori modification
  float periodDistance = 4 * maxDisplacement;
  modification = fmod(periodDistance / revolutionTime * deltaTime, periodDistance);

  // Wrap it around the max displacement values
  bool wrapped;
  do
  {
    wrapped = false;

    if (displacementDirection > 0)
    {
      if (maxDisplacement - displacement < modification)
      {
        modification -= maxDisplacement - displacement;
        displacement = maxDisplacement;
        displacementDirection = -1;
        wrapped = true;
      }
    }
    else
    {
      if (displacement - (-maxDisplacement) < modification)
      {
        modification -= displacement - (-maxDisplacement);
        displacement = -maxDisplacement;
        displacementDirection = 1;
        wrapped = true;
      }
    }
  } while (wrapped);

  // Calculate new displacement
  displacement += modification * displacementDirection;

  LOCK(weakRenderer, renderer);
  float currentAngle = renderer->worldObject.GetScale().x < 0 ? M_PI - angle : angle;

  // Apply it
  SetOffset(originalSpriteOffset + Vector2::Angled(currentAngle, displacement));
}
