#ifndef __PARTICLE_SYSTEM__
#define __PARTICLE_SYSTEM__

#include "Vector2.h"
#include "Color.h"
#include "Particle.h"
#include <memory>
#include <unordered_map>

class GameState;

class ParticleSystem
{
public:
  ParticleSystem(GameState &gameState);

  void PhysicsUpdate(float deltaTime);

  // Deletes a particle
  void DeleteParticle(int id);

  // Creates a particle
  std::shared_ptr<Particle> CreateParticle(Vector2 position, float lifetime, Vector2 velocity = Vector2::Zero(), Color color = Color::White());

private:
  GameState &gameState;

  // Particle id generator
  int nextId{0};

  // Store all particles
  std::unordered_map<int, std::shared_ptr<Particle>> particles;
};

#endif