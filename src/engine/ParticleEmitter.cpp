#include "Helper.h"
#include "ParticleEmitter.h"

using namespace std;
using namespace Helper;

ParticleEmitter::ParticleEmitter(GameObject &associatedObject, RenderLayer renderLayer, float radius, bool loop, float duration)
    : Component(associatedObject),
      duration(duration),
      loop(loop),
      // Default parameters:
      emissionFrequency({0.1f, 0.5f}),
      emissionAngle({0, 2 * M_PI}),
      emissionColor({Color::White(), Color::White()}),
      emissionSpeed({0.5f, 2}),
      emissionLifetime({1, 2}),
      origin(radius),
      particleSystem(gameState.particleSystem),
      renderLayer(renderLayer)
{
}

void ParticleEmitter::SetOffset(Vector2 offset) { origin.center = offset; }
void ParticleEmitter::SetRadius(float radius) { origin.radius = radius; }

Circle ParticleEmitter::GetOrigin() const { return origin; }

void ParticleEmitter::Render()
{
  // Render each of this emitter's particles
  for (auto particle : GetEmittedParticles())
    particle->Render();
}

void ParticleEmitter::Start()
{
  if (emitOnStart)
    StartEmission();
}

void ParticleEmitter::Update(float deltaTime)
{
  if (active == false)
    return;

  // Count emission time
  emitCooldown -= deltaTime;

  if (emitCooldown <= 0)
  {
    // Restart timer
    emitCooldown += RandomRange(emissionFrequency);

    Emit();
  }

  // Count cycleLifetime
  cycleLifetime -= deltaTime;

  // Check for cycle end
  if (cycleLifetime <= 0)
  {
    if (loop)
      cycleLifetime += duration;
    else
    {
      Stop();
      return;
    }
  }
}

void ParticleEmitter::Emit()
{
  // Get base parameters
  float angle = RandomRange(emissionAngle);
  Color color = RandomRange(emissionColor);
  float lifetime = RandomRange(emissionLifetime);
  float speed = RandomRange(emissionSpeed);

  // Get position
  float positionAngle = RandomRange(0.0f, 2 * M_PI);
  float positionRadius = RandomRange(0.0f, origin.radius);
  Vector2 position = origin.center + Vector2::Angled(positionAngle, positionRadius);

  // Create particle
  auto particle = particleSystem.CreateParticle(position, lifetime, Vector2::Angled(angle, speed), color);

  // Attachment
  if (attachToEmitter)
    particle->AttachTo(gameObject.GetShared());
}

void ParticleEmitter::StartEmission()
{
  cycleLifetime = duration;
  active = true;
  emitCooldown = 0;
}

void ParticleEmitter::Stop()
{
  cycleLifetime = 0;
  active = false;
}

list<shared_ptr<Particle>> ParticleEmitter::GetEmittedParticles()
{
  return ParseWeakIntoShared(weakEmittedParticles);
}
