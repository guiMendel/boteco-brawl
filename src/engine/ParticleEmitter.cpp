#include "Helper.h"
#include "Debug.h"
#include "ParticleEmitter.h"

using namespace std;
using namespace Helper;

ParticleEmitter::ParticleEmitter(GameObject &associatedObject, RenderLayer renderLayer, float radius, bool loop, float duration)
    : Component(associatedObject),
      duration(duration),
      loop(loop),
      origin(radius),
      renderLayer(renderLayer)
{
  // Default parameters:
  emission.frequency = {0.1f, 0.5f};
  emission.angle = {0, 2 * M_PI};
  emission.color = {Color::White(), Color::White()};
  emission.speed = {0.5f, 2};
  emission.lifetime = {1, 2};
}

void ParticleEmitter::SetOffset(Vector2 offset) { origin.center = offset; }
void ParticleEmitter::SetRadius(float radius) { origin.radius = radius; }

Circle ParticleEmitter::GetOrigin() const { return origin; }

void ParticleEmitter::Render()
{
  // cout << gameObject.GetPosition() << endl;

  // Debug
  Debug::DrawCircle(GetOrigin() + gameObject.GetPosition(), Color::Yellow());

  // Render each of this emitter's particles
  for (auto particle : GetEmittedParticles())
    particle->Render();
}

void ParticleEmitter::Start()
{
  if (emitOnStart)
    StartEmission();
}

void ParticleEmitter::PhysicsUpdate(float deltaTime)
{
  if (active == false)
    return;

  // Count emission time
  emitCooldown -= deltaTime;

  while (emitCooldown <= 0)
  {
    // Restart timer
    emitCooldown += RandomRange(currentParams.frequency);

    Emit();
  }

  // Call evolution callback
  if (emissionEvolution)
    emissionEvolution(currentParams, deltaTime);

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
  float angle = RandomRange(currentParams.angle);
  Color color = RandomRange(currentParams.color);
  float lifetime = RandomRange(currentParams.lifetime);
  float speed = RandomRange(currentParams.speed);

  // Get position
  float positionAngle = RandomRange(0.0f, 2 * M_PI);
  float positionRadius = RandomRange(0.0f, origin.radius);
  Vector2 position = gameObject.GetPosition() + origin.center + Vector2::Angled(positionAngle, positionRadius);

  // Create particle
  auto particle = GetSystem().CreateParticle(position, lifetime, Vector2::Angled(angle, speed), color);

  // Attachment
  if (attachToEmitter)
    particle->AttachTo(gameObject.GetShared());

  // Register it
  weakEmittedParticles.emplace_back(particle);
}

void ParticleEmitter::StartEmission()
{
  cycleLifetime = duration;
  active = true;
  emitCooldown = 0;

  // Reset params
  currentParams = emission;
}

void ParticleEmitter::Stop()
{
  cycleLifetime = 0;
  active = false;
  OnStop.Invoke();
}

list<shared_ptr<Particle>> ParticleEmitter::GetEmittedParticles()
{
  return ParseWeakIntoShared(weakEmittedParticles);
}

ParticleSystem &ParticleEmitter::GetSystem() const
{
  return GetState()->particleSystem;
}
