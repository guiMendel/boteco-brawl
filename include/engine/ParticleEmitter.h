#ifndef __PARTICLE_EMITTER__
#define __PARTICLE_EMITTER__

#include "GameObject.h"
#include "Component.h"
#include "Circle.h"
#include "Color.h"
#include "ParticleSystem.h"
#include <utility>
#include <functional>
#include <list>

class Particle;

struct ParticleEmissionParameters
{
  template <class T>
  using range = std::pair<T, T>;

  // How often particles are emitted, in seconds
  range<float> frequency;

  // Angle of emitted particles, in radians
  range<float> angle;

  // Color of emitted particles
  range<Color> color;

  // Speed of emitted particles, in units / second
  range<float> speed;

  // Lifetime of emitted particles, in seconds
  range<float> lifetime;
};

class ParticleEmitter : public Component
{
public:
  ParticleEmitter(GameObject &associatedObject, RenderLayer renderLayer = RenderLayer::Default, float radius = 0.01, bool loop = false, float duration = 1);

  ~ParticleEmitter() {}

  void SetOffset(Vector2 offset);

  void SetRadius(float radius);

  Circle GetOrigin() const;

  void Start() override;
  void PhysicsUpdate(float deltaTime) override;
  void Render() override;
  RenderLayer GetRenderLayer() override { return renderLayer; }
  int GetRenderOrder() override { return renderOrder; }

  // Start an emission cycle
  void StartEmission();

  // Stop emission
  void Stop();

  // How many seconds to emit for
  float duration;

  // Whether to loop when emission cycle ends
  bool loop;

  // Initial parameters at each cycle
  ParticleEmissionParameters emission;

  // Frame behavior to attach to emitted particles
  std::function<void(Particle &)> particleBehavior{nullptr};

  // Permits altering the emission params over an emission cycle
  // Called each frame
  std::function<void(ParticleEmissionParameters &, float)> emissionEvolution{nullptr};

  // Whether to simulate particle with reference to this emitter or the world
  bool attachToEmitter{false};

  // Whether to emit on start hook
  bool emitOnStart{true};

private:
  void Emit();

  // Gets a list of all active particles emitted by this emitter
  std::list<std::shared_ptr<Particle>> GetEmittedParticles();

  // Circle from which particles will be emitted
  // Its coordinates act as an offset from the object's position
  Circle origin;

  // Whether is currently emitting
  bool active{false};

  // Seconds the current cycle still has to finish
  float cycleLifetime{0};

  // Time until next emission
  float emitCooldown{0};

  // Reference to particle system
  ParticleSystem &particleSystem;

  // Which particles it has emitted
  std::list<std::weak_ptr<Particle>> weakEmittedParticles;

  // Layer to render to
  RenderLayer renderLayer;

  // Render order
  int renderOrder{0};

  // Current value of params in the current cycle
  ParticleEmissionParameters currentParams;
};

namespace ParticleBehaviors
{
  static void Oscillate(Particle &);
}

#endif