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
  Particle(ParticleSystem &particleSystem, int id, Vector2 position, float lifetime, Vector2 velocity, Color color);

  void PhysicsUpdate(float deltaTime);

  void Render();

  // Attach position to a gameObject
  void AttachTo(std::shared_ptr<GameObject> gameObject);

  // Function to execute on update cycles
  void SetBehavior(std::function<void(Particle &)> newBehavior);

  Vector2 position;
  Vector2 velocity;
  Color color;
  float lifetime;

private:
  ParticleSystem &particleSystem;

  // Reference to simulated position attachment to
  std::weak_ptr<GameObject> referenceObject;

  // Physics update individual behavior
  std::function<void(Particle &)> behavior;

  // This particles unique identifier
  int id;
};

#endif