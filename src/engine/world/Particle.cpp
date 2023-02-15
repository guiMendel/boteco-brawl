#include "Particle.h"
#include "Camera.h"
#include "ParticleSystem.h"
#include "Game.h"
#include <SDL_image.h>

using namespace std;

Particle::Particle(ParticleSystem &particleSystem, int id, Vector2 position, float lifetime, Vector2 velocity, Vector2 gravityModifier, Color color)
    : position(position), velocity(velocity), gravityModifier(gravityModifier), color(color), lifetime(lifetime), particleSystem(particleSystem), id(id)
{
}

void Particle::PhysicsUpdate(float deltaTime)
{
  const static auto &physicsSystem = particleSystem.gameScene.physicsSystem;

  // Use time scale from reference
  IF_LOCK(weakReferenceObject, referenceObject)
  {
    deltaTime = deltaTime * referenceObject->GetTimeScale();
  }

  // Apply behavior
  if (behavior)
    behavior(*this, deltaTime);

  // Update velocity
  velocity += physicsSystem.gravity * gravityModifier * deltaTime;

  // Update position
  position += deltaTime * velocity;

  // Count lifetime
  if ((lifetime -= deltaTime) <= 0)
    deleteRequested = true;
}

// Attach position to a worldObject
void Particle::AttachTo(std::shared_ptr<WorldObject> worldObject)
{
  weakReferenceObject = worldObject;
}

void Particle::Render()
{
  // Get camera
  auto camera = Camera::GetMain();

  // Size of virtual pixel, in units
  float virtualPixelSize = 1.0f / Game::defaultVirtualPixelsPerUnit;

  // Size of virtual pixel, in real pixels
  float pixelSize = camera->GetRealPixelsPerUnit() * virtualPixelSize;

  // Get rect representing this particle
  SDL_Rect pixel = (SDL_Rect)Rectangle{camera->WorldToScreen(position), pixelSize, pixelSize};

  // Get renderer
  auto renderer = Game::GetInstance().GetRenderer();

  // Use color
  SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, color.alpha);

  // Fill a rect at this particle's position
  SDL_RenderFillRect(renderer, &pixel);
}

bool Particle::DeleteRequested() const { return deleteRequested; }
