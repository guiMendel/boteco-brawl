#ifndef __CHARACTER_VFX__
#define __CHARACTER_VFX__

#include "WorldComponent.h"
#include "ParticleEmitter.h"
#include "ParticleFX.h"
#include "Rigidbody.h"

#define PARTICLE_EMITTER_OBJECT "ParticleEffects"

class CharacterVFX : public WorldComponent
{
public:
  template <class T>
  using range = ParticleEmissionParameters::range<T>;

  CharacterVFX(GameObject &associatedObject);
  virtual ~CharacterVFX() {}

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
  std::weak_ptr<Rigidbody> weakBody;
};

#endif