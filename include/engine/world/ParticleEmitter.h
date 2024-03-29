#ifndef __PARTICLE_EMITTER__
#define __PARTICLE_EMITTER__

#include "WorldObject.h"
#include "WorldComponent.h"
#include "Circle.h"
#include "Color.h"
#include "ParticleSystem.h"
#include "Particle.h"
#include <utility>
#include <functional>
#include <list>

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

  // Modifier applied to particle's gravity
  range<Vector2> gravityModifier;

  // Allows changing particle's properties over time
  Particle::behavior_callback behavior;

  ParticleEmissionParameters();
};

class ParticleEmitter : public WorldComponent
{
public:
  Event OnStop;

  ParticleEmitter(GameObject &associatedObject,
                  RenderLayer renderLayer = RenderLayer::Default,
                  std::unique_ptr<Shape> origin = std::make_unique<Circle>(0.01),
                  bool loop = false,
                  float duration = 1);

  virtual ~ParticleEmitter() {}

  void Start() override;
  void PhysicsUpdate(float deltaTime) override;
  void Render() override;
  RenderLayer GetRenderLayer() override { return renderLayer; }
  int GetRenderOrder() override { return renderOrder; }

  // Start an emission cycle
  void StartEmission();

  // Stop emission
  void Stop();

  bool IsEmitting() const;

  // How many seconds to emit for
  float duration;

  // Whether to always pick direction outwards from center (sets emission direction as unused)
  bool irradiateParticles{false};

  // Whether to loop when emission cycle ends
  bool loop;

  // Initial parameters at each cycle
  ParticleEmissionParameters emission;

  // Permits altering the emission params over an emission cycle
  // Called each frame
  std::function<void(ParticleEmissionParameters &, float)> emissionEvolution{nullptr};

  // Whether to simulate particle with reference to this emitter or the world
  // bool positionOnEmitter{false};

  // Whether to emit on start hook
  bool emitOnStart{true};

  // Render order
  int renderOrder{0};

  // Circle from which particles will be emitted
  // Its coordinates act as an offset from the object's position
  std::unique_ptr<Shape> origin;

private:
  void Emit();

  // Get the particle system
  ParticleSystem &GetSystem() const;

  // Gets a list of all active particles emitted by this emitter
  std::list<std::shared_ptr<Particle>> GetEmittedParticles();

  // Whether is currently emitting
  bool active{false};

  // Seconds the current cycle still has to finish
  float cycleLifetime{0};

  // Time until next emission
  float emitCooldown{0};

  // Which particles it has emitted
  std::list<std::weak_ptr<Particle>> weakEmittedParticles;

  // Layer to render to
  RenderLayer renderLayer;

  // Current value of params in the current cycle
  ParticleEmissionParameters currentParams;
};

#endif