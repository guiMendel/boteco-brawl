#ifndef __CHARACTER_VFX__
#define __CHARACTER_VFX__

#include "Component.h"
#include "ParticleEmitter.h"
#include "ParticleFX.h"

#define PARTICLE_EMITTER_OBJECT "ParticleEffects"

class CharacterVFX : public Component
{
public:
  template <class T>
  using range = ParticleEmissionParameters::range<T>;

  CharacterVFX(GameObject &associatedObject);
  virtual ~CharacterVFX() {}

  // void Update(float) override;
  void Awake() override;

  // Plays dust particles
  void PlayDust(Vector2 offset, range<float> angle = {0, -M_PI}, range<float> speed = {3, 8});

  // Plays sparks particles
  void PlaySparks(Vector2 offset, range<float> frequency, float angle = DegreesToRadians(30), range<float> speed = {-25, 25});

  void StartDash();
  void StopDash();

private:
  std::weak_ptr<ParticleEmitter> weakDashEmitter;
  std::weak_ptr<ParticleEmitter> weakSmokeEmitter;
};

#endif