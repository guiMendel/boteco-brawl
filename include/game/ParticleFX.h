#ifndef __PARTICLE_FX__
#define __PARTICLE_FX__

#include "GameObject.h"
#include "Component.h"
#include "ParticleEmitter.h"
#include <unordered_map>

class ParticleFX : public Component
{
public:
  ParticleFX(GameObject &associatedObject);
  virtual ~ParticleFX() {}

  // Spawns a particle emitter to play the given effect, discards the emitter object after duration + destroyAfter seconds
  void PlayEffectAt(Vector2 position, float radius, float duration, ParticleEmissionParameters params, float destroyAfter = 0);

  // Finds the particle FX instance and plays on it
  static void EffectAt(Vector2 position, float radius, float duration, ParticleEmissionParameters params, float destroyAfter = 0);

  void Update(float) override;

private:
  // How much time to wait before destroying each child
  std::unordered_map<int, float> childrenDestroyTimers;
};

#endif