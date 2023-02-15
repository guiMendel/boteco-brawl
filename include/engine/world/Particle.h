#ifndef __PARTICLE__
#define __PARTICLE__

#include "Vector2.h"
#include "Color.h"
#include <memory>
#include <functional>

class ParticleSystem;
class WorldObject;

class Particle
{
  friend class ParticleSystem;

public:
  using behavior_callback = std::function<void(Particle&, float)>;

  Particle(
      ParticleSystem &particleSystem, int id, Vector2 position,
      float lifetime, Vector2 velocity, Vector2 gravityModifier, Color color);

  void PhysicsUpdate(float deltaTime);

  void Render();

  // Attach position to a worldObject
  void AttachTo(std::shared_ptr<WorldObject> worldObject);

  // Whether this particle has requested deletion
  bool DeleteRequested() const;

  Vector2 position;
  Vector2 velocity;
  Vector2 gravityModifier;
  Color color;
  float lifetime;

  behavior_callback behavior;

private:
  ParticleSystem &particleSystem;

  // Reference to simulated position attachment to
  std::weak_ptr<WorldObject> weakReferenceObject;

  // Whether this particle should be deleted asap
  bool deleteRequested{false};

  // This particles unique identifier
  int id;
};

#endif