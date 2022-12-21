#include "ParticleSystem.h"

using namespace std;

ParticleSystem::ParticleSystem(GameState &gameState) : gameState(gameState) {}

shared_ptr<Particle> ParticleSystem::CreateParticle(Vector2 position, float lifetime, Vector2 velocity, Color color)
{
  auto particle = make_shared<Particle>(*this, nextId++, position, lifetime, velocity, color);

  // Register it
  particles[particle->id] = particle;

  return particle;
}

void ParticleSystem::PhysicsUpdate(float deltaTime)
{
  // cout << particles.size() << " particles" << endl;

  // Update each particle
  for (auto [id, particle] : particles)
    particle->PhysicsUpdate(deltaTime);
}

void ParticleSystem::DeleteParticle(int id) { particles.erase(id); }