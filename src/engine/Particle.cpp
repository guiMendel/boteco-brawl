#include "Particle.h"
#include "Camera.h"
#include "ParticleSystem.h"
#include "Game.h"
#include <SDL_image.h>

Particle::Particle(ParticleSystem &particleSystem, int id, Vector2 position, float lifetime, Vector2 velocity, Color color)
    : position(position), velocity(velocity), color(color), lifetime(lifetime), particleSystem(particleSystem), id(id)
{
}

void Particle::PhysicsUpdate(float deltaTime)
{
  // Update position
  position += deltaTime * velocity;

  // Perform individual behavior
  if (behavior)
    behavior(*this);

  // Count lifetime
  if ((lifetime -= deltaTime) <= 0)
    particleSystem.DeleteParticle(id);
}

// Attach position to a gameObject
void Particle::AttachTo(std::shared_ptr<GameObject> gameObject)
{
  referenceObject = gameObject;
}

// Function to execute on update cycles
void Particle::SetBehavior(std::function<void(Particle &)> newBehavior)
{
  behavior = newBehavior;
}

void Particle::Render()
{
  // Get camera
  auto camera = Camera::GetMain();

  // Size of virtual pixel, in units
  float virtualPixelSize = 1 / Game::defaultVirtualPixelsPerUnit;

  // Size of virtual pixel, in real pixels
  float pixelSize = camera->GetRealPixelsPerUnit() * virtualPixelSize;

  // Get rect representing this particle
  SDL_Rect pixel = (SDL_Rect)Rectangle{camera->WorldToScreen(position), pixelSize, pixelSize};

  // Fill a rect at this particle's position
  SDL_RenderFillRect(Game::GetInstance().GetRenderer(), &pixel);
}
