#ifndef __PARTICLE_SYSTEM__
#define __PARTICLE_SYSTEM__

#include "Vector2.h"
#include "Color.h"
#include "Particle.h"
#include <memory>
#include <unordered_map>

class GameScene;

class ParticleSystem
{
  friend class Particle;

public:
  ParticleSystem(GameScene &gameScene);

  void PhysicsUpdate(float deltaTime);

  // Deletes a particle
  void DeleteParticle(int id);

  // Creates a particle
  std::shared_ptr<Particle> CreateParticle(Vector2 position, float lifetime, Vector2 velocity = Vector2::Zero(), Color color = Color::White(), Vector2 gravityModifier = Vector2::Zero(), Particle::behavior_callback behavior = nullptr);

private:
  GameScene &gameScene;

  // Particle id generator
  int nextId{0};

  // Store all particles
  std::unordered_map<int, std::shared_ptr<Particle>> particles;
};

#endif