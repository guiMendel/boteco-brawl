#include "Helper.h"
#include "Debug.h"
#include "ParticleEmitter.h"

// #define RENDER_EMITTER_OUTLINE

using namespace std;
using namespace Helper;

ParticleEmissionParameters::ParticleEmissionParameters()
    : frequency({0.1f, 0.5f}),
      angle({0, 2 * M_PI}),
      color({Color::White(), Color::White()}),
      speed({0.5f, 2}),
      lifetime({1, 2}),
      gravityModifier({Vector2::Zero(), Vector2::Zero()}) {}

ParticleEmitter::ParticleEmitter(GameObject &associatedObject, RenderLayer renderLayer, unique_ptr<Shape> origin, bool loop, float duration)
    : WorldComponent(associatedObject),
      duration(duration),
      loop(loop),
      origin(origin.release()),
      renderLayer(renderLayer) {}

void ParticleEmitter::Render()
{
#ifdef RENDER_EMITTER_OUTLINE
  // Debug
  origin->DebugDrawAt(worldObject.GetPosition(), Color::Yellow());
#endif

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
  Vector2 gravityModifier = RandomRange(currentParams.gravityModifier);

  // Get position
  Vector2 position = worldObject.GetPosition() + origin->SamplePoint();

  // Create particle
  auto particle = GetSystem().CreateParticle(
      position, lifetime, Vector2::Angled(angle, speed), color, gravityModifier, currentParams.behavior);

  // Attachment
  if (attachToEmitter)
    particle->AttachTo(worldObject.GetShared());

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
  if (active == false)
    return;

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
  return GetScene()->particleSystem;
}

bool ParticleEmitter::IsEmitting() const { return active; }
