#ifndef __SHAKE_EFFECT_MANAGER__
#define __SHAKE_EFFECT_MANAGER__

#include "Component.h"

class ShakeEffectManager;

// Struct that stores and handles each individual active shake effect
struct ShakeEffect
{
  ShakeEffect();
  
  ShakeEffect(std::shared_ptr<GameObject> target,
              ShakeEffectManager &manager,
              float angle,
              std::pair<float, float> displacementEvolution,
              std::pair<float, float> revolutionTimeEvolution,
              float effectDuration,
              float stopDuration);

  ~ShakeEffect();

  void Update(float);

  // Starts stop time
  void Stop();

  // Resets any modifications
  void Reset();

  // Set an offset to the renderer
  // Automatically avoids reacting to the new offset value that will be raised in consequence
  void SetOffset(Vector2 value);

  ShakeEffectManager &manager;

  // Sprite renderer of affected object
  std::weak_ptr<SpriteRenderer> weakRenderer;

  // Angle of shaking
  float angle;

  // Effect's time to live
  float timeToLive;

  // Current displacement applied to renderer
  float displacement;

  // Original offset of the renderer's current sprite
  Vector2 originalSpriteOffset;

  // Current value of max displacement to apply
  float maxDisplacement;

  // Current value of revolution time
  float revolutionTime;

  // Speed of change to maxDisplacement
  float maxDisplacementChange;

  // Speed of change to revolution time
  float revolutionTimeChange;

  // How long to take to stabilize displacement back to 0
  float stopDuration;

  // Whether is currently in stop time
  bool inStopTime{false};

  // Whether to ignore renderers new offset event
  bool ignoreNewOffsetEvent{false};

  // Whether is currently increasing displacement
  bool increasingDisplacement{true};
};

class ShakeEffectManager : public Component
{
  friend struct ShakeEffect;

public:
  ShakeEffectManager(GameObject &associatedObject);
  virtual ~ShakeEffectManager() {}

  void PhysicsUpdate(float) override;

  // Apply a shake effect to an object for a given time
  void Shake(std::shared_ptr<GameObject> target,
             float angle,
             std::pair<float, float> displacementEvolution,
             std::pair<float, float> revolutionTimeEvolution,
             float effectDuration,
             float stopDuration = 0.5);

  // Stop shaking a given object
  void StopShake(std::shared_ptr<GameObject> target, float overrideStopDuration = -1);

private:
  // Removes effect and resets any of it's modifications
  void EraseShake(int targetId);

  // Associates the id of an object with an active shake to the shake effect
  std::unordered_map<int, std::unique_ptr<ShakeEffect>> activeShakes;
};

#endif