#ifndef __PARTICLE__
#define __PARTICLE__

#include "Vector2.h"
#include "Color.h"
#include <memory>
#include <functional>

class ParticleSystem;
class GameObject;

class Particle
{
  friend class ParticleSystem;

public:
  Particle(
      ParticleSystem &particleSystem, int id, Vector2 position,
      float lifetime, Vector2 velocity, Vector2 gravityModifier, Color color);

  void PhysicsUpdate(float deltaTime);

  void Render();

  // Attach position to a gameObject
  void AttachTo(std::shared_ptr<GameObject> gameObject);

  // Whether this particle has requested deletion
  bool DeleteRequested() const;

  Vector2 position;
  Vector2 velocity;
  Vector2 gravityModifier;
  Color color;
  float lifetime;

  // std::function<void(Particle&, float)>

private:
  ParticleSystem &particleSystem;

  // Reference to simulated position attachment to
  std::weak_ptr<GameObject> referenceObject;

  // Whether this particle should be deleted asap
  bool deleteRequested{false};

  // This particles unique identifier
  int id;
};

#endif