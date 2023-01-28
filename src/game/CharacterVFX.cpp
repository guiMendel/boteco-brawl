#include "CharacterVFX.h"
#include "Heat.h"
#include "BoxCollider.h"

// Minimum speed to start emitting smoke
static const float minSmokeSpeed{6};

CharacterVFX::CharacterVFX(WorldObject &associatedObject) : Component(associatedObject) {}

void CharacterVFX::Awake()
{
  weakBody = worldObject.RequireComponent<Rigidbody>();

  auto emitterObject = worldObject.CreateChild(PARTICLE_EMITTER_OBJECT);
  auto characterBox = worldObject.RequireComponent<BoxCollider>()->GetBox();

  // Configure dash emitter
  auto dashEmitter = emitterObject->AddComponent<ParticleEmitter>(
      RenderLayer::VFX, characterBox.GetMinDimension() / 2, true);

  weakDashEmitter = dashEmitter;

  dashEmitter->emission.color = {Color::White(), Color::Gray()};
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
  smokeEmitter->emission.color = {{255, 255, 255, 125}, Color::Gray()};
  smokeEmitter->emission.frequency = {0.01, 0.005};
  smokeEmitter->emission.speed = {0.05, 0.5};
  smokeEmitter->emission.lifetime = {0.5, 1.4};
  smokeEmitter->emission.gravityModifier = {Vector2::Up(0.05), Vector2::Zero()};
  smokeEmitter->emitOnStart = false;
  smokeEmitter->emissionEvolution = [this](ParticleEmissionParameters &params, float)
  {
    LOCK(weakBody, body);

    float sqrSpeed = body->velocity.SqrMagnitude();

    if (sqrSpeed < minSmokeSpeed * minSmokeSpeed)
    {
      LOCK(weakSmokeEmitter, smokeEmitter);

      smokeEmitter->Stop();
    }

    else
      params.frequency = {1 / sqrSpeed, 1 / sqrSpeed / 3};
  };

  auto emitSmoke = [this](Damage, float)
  {
    LOCK(weakSmokeEmitter, smokeEmitter);

    smokeEmitter->StartEmission();
  };

  worldObject.RequireComponent<Heat>()->OnTakeDamage.AddListener("start-smoke-particles", emitSmoke);
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

  if (worldObject.localScale.x < 0)
  {
    offset.x *= -1;
    emission.angle.first = M_PI - emission.angle.first;
    emission.angle.second = M_PI - emission.angle.second;
  }

  ParticleFX::EffectAt(worldObject.GetPosition() + offset, 0.1, 0.1, emission, 3.0);
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

  if (worldObject.localScale.x < 0)
  {
    offset.x *= -1;
    emission.angle.first = M_PI - emission.angle.first;
    emission.angle.second = M_PI - emission.angle.second;
  }

  ParticleFX::EffectAt(worldObject.GetPosition() + offset, 0.1, 0.1, emission, 0.2);
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
