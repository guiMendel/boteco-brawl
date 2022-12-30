#include "ParticleSystem.h"

using namespace std;

ParticleSystem::ParticleSystem(GameState &gameState) : gameState(gameState) {}

shared_ptr<Particle> ParticleSystem::CreateParticle(
    Vector2 position, float lifetime, Vector2 velocity, Color color, Vector2 gravityModifier)
{
  auto particle = make_shared<Particle>(*this, nextId++, position, lifetime, velocity, gravityModifier, color);

  // Register it
  particles[particle->id] = particle;

  return particle;
}

void ParticleSystem::PhysicsUpdate(float deltaTime)
{
  // cout << particles.size() << " particles" << endl;

  // Update each particle
  for (auto [id, particle] : particles)
  {
    particle->PhysicsUpdate(deltaTime);
  }

  // Delete dead particles
  auto particleIterator = particles.begin();
  while (particleIterator != particles.end())
  {
    if (particleIterator->second->DeleteRequested())
      particleIterator = particles.erase(particleIterator);
    else
      particleIterator++;
  }
}

void ParticleSystem::DeleteParticle(int id) { particles.erase(id); }