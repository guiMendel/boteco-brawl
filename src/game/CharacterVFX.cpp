#include "CharacterVFX.h"
#include "BoxCollider.h"

CharacterVFX::CharacterVFX(GameObject &associatedObject) : Component(associatedObject) {}

void CharacterVFX::Awake()
{
  auto emitterObject = gameObject.CreateChild(PARTICLE_EMITTER_OBJECT);
  auto characterBox = gameObject.RequireComponent<BoxCollider>()->GetBox();

  // Configure dash emitter
  auto dashEmitter = emitterObject->AddComponent<ParticleEmitter>(
      RenderLayer::VFX, characterBox.GetMinDimension() / 2, true);

  weakDashEmitter = dashEmitter;

  dashEmitter->SetOffset({0, 0.25});
  dashEmitter->emission.color = {Color::Black(), Color::Gray()};
  dashEmitter->emission.frequency = {0.0001, 0.001};
  dashEmitter->emission.speed = {0.01, 0.1};
  dashEmitter->emission.lifetime = {0.2, 0.6};
  dashEmitter->emitOnStart = false;
  // Reduce frequency over emission
  dashEmitter->emissionEvolution = [](ParticleEmissionParameters &params, float deltaTime)
  {
    auto reduction = 0.5 * deltaTime;
    params.frequency = {params.frequency.first + reduction,
                        params.frequency.second + reduction};
  };

  // Configure smoke emitter
  auto smokeEmitter = emitterObject->AddComponent<ParticleEmitter>(
      RenderLayer::VFX, characterBox.GetMinDimension() / 2, true);

  weakSmokeEmitter = smokeEmitter;

  smokeEmitter->emitOnStart = false;
}

void CharacterVFX::PlayDust(Vector2 offset, range<float> angle, range<float> speed)
{
  ParticleEmissionParameters emission;
  emission.angle = angle;
  emission.speed = speed;
  emission.color = {Color::Black(), Color::Gray()};
  emission.frequency = {0.0005, 0.02};
  emission.lifetime = {0.2, 1.0};
  emission.gravityModifier = {Vector2::One(), Vector2::One()};

  if (gameObject.localScale.x < 0)
  {
    offset.x *= -1;
    emission.angle.first = M_PI - emission.angle.first;
    emission.angle.second = M_PI - emission.angle.second;
  }

  ParticleFX::EffectAt(gameObject.GetPosition() + offset, 0.1, 0.1, emission, 3.0);
}

void CharacterVFX::PlaySparks(Vector2 offset, range<float> frequency, float effectArc, range<float> speed)
{
  // Get random angle
  float angleCenter = RandomRange(0, 2 * M_PI);

  ParticleEmissionParameters emission;
  emission.color = {Color::Yellow(), Color::White()};
  emission.speed = speed;
  emission.frequency = frequency;
  emission.lifetime = {0.01, 0.1};
  emission.angle = {angleCenter + effectArc / 2, angleCenter - effectArc / 2};

  if (gameObject.localScale.x < 0)
  {
    offset.x *= -1;
    emission.angle.first = M_PI - emission.angle.first;
    emission.angle.second = M_PI - emission.angle.second;
  }

  ParticleFX::EffectAt(gameObject.GetPosition() + offset, 0.1, 0.1, emission, 0.2);
}

void CharacterVFX::StartDash()
{
  LOCK(weakDashEmitter, dashEmitter);

  dashEmitter->StartEmission();
}

void CharacterVFX::StopDash()
{
  LOCK(weakDashEmitter, dashEmitter);

  dashEmitter->Stop();
}
