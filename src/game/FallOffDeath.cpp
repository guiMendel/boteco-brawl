#include "FallOffDeath.h"
#include "Animator.h"
#include "Movement.h"
#include "PlayerInput.h"
#include "CharacterController.h"
#include "CharacterStateManager.h"
#include "ParticleFX.h"

#define RESPAWN_TIMER "respawn"

using namespace std;

const float FallOffDeath::deathMargin{3};
const float FallOffDeath::respawnDelay{0.1};

FallOffDeath::FallOffDeath(GameObject &associatedObject)
    : Component(associatedObject), weakArena(GetState()->FindObjectOfType<Arena>())
{
  Assert(weakArena.expired() == false, "Failed to find an Arena component");
}

void FallOffDeath::Update(float)
{
  LOCK(weakArena, arena);

  if (IsDead())
  {
    // If respawn timer is up, respawn
    if (gameObject.timer.Get(RESPAWN_TIMER) >= respawnDelay)
      Respawn();

    return;
  }

  // Otherwise, check for death
  auto position = gameObject.GetPosition();

  if (
      // Off left
      position.x < -arena->width / 2 - deathMargin ||
      // Off right
      position.x > arena->width / 2 + deathMargin ||
      // Off top
      position.y < -arena->height / 2 - deathMargin ||
      // Off bottom
      position.y > arena->height / 2 + deathMargin)
    Die();
}

void FallOffDeath::Die()
{
  LOCK(weakArena, arena);

  dead = true;

  OnDeath.Invoke();

  // Disable character
  SetCharacterActive(false);

  // Start respawn timer
  gameObject.timer.Start(RESPAWN_TIMER);

  // Get radius
  auto radius = gameObject.RequireComponent<Collider>()->DeriveShape()->GetMaxDimension();

  // Get effect position
  Vector2 effectPosition{
      Clamp(gameObject.GetPosition().x, -arena->width / 2 - radius, arena->width / 2 + radius),
      Clamp(gameObject.GetPosition().y, -arena->height / 2 - radius, arena->height / 2 + radius)};

  // Get effect direction
  Vector2 effectDirection;
  if (gameObject.GetPosition().x < -arena->width / 2)
    effectDirection.x = 1;
  else if (gameObject.GetPosition().x > arena->width / 2)
    effectDirection.x = -1;

  if (gameObject.GetPosition().y < -arena->height / 2)
    effectDirection.y = 1;
  else if (gameObject.GetPosition().y > arena->height / 2)
    effectDirection.y = -1;

  float effectAngle = effectDirection.Angle();

  // Fire effect params
  ParticleEmissionParameters fireParams;
  fireParams.angle = {effectAngle - DegreesToRadians(3), effectAngle + DegreesToRadians(3)};
  fireParams.color = {Color::Red(), Color::Yellow()};
  fireParams.frequency = {0.00001, 0.0005};
  fireParams.gravityModifier = {Vector2::One() / 8, -Vector2::One() / 8};
  fireParams.lifetime = {0.01, 0.3};
  fireParams.speed = {10, 40};
  // Make it turn to smoke slowly
  fireParams.behavior = [](Particle &particle, float deltaTime)
  {
    particle.color = Color::ClampValid(particle.color - Color(900, 900, 900, 0) * deltaTime);
  };

  // Smoke effect params
  ParticleEmissionParameters smokeParams;
  smokeParams.angle = fireParams.angle;
  smokeParams.color = {Color::Black(), Color::Gray()};
  smokeParams.frequency = {0.0001, 0.005};
  smokeParams.gravityModifier = {-Vector2::One() / 10, -Vector2::One() / 5};
  smokeParams.lifetime = {0.1, 1};
  smokeParams.speed = {1, 10};

  // Arc effect
  ParticleEmissionParameters arcParams;
  arcParams.color = {Color::Red(), Color::White()};
  arcParams.frequency = {0.000005, 0.00005};
  arcParams.lifetime = {0.6, 1.5};
  arcParams.speed = {10, 10};

  // Play effect
  ParticleFX::EffectAt(effectPosition, radius, 0.5, fireParams, 5);
  ParticleFX::EffectAt(effectPosition, radius, 0.5, smokeParams, 5);
  ParticleFX::EffectAt(effectPosition, 0.1, 0.001, arcParams, 10);

  // Relocate character
  gameObject.SetPosition({0, 0});
}

void FallOffDeath::Respawn()
{
  LOCK(weakArena, arena);

  dead = false;

  // Reset timer
  gameObject.timer.Reset(RESPAWN_TIMER, 0, false);

  // Get height
  auto height = gameObject.RequireComponent<Collider>()->DeriveShape()->GetMaxDimension();

  // Set new position
  gameObject.SetPosition({0, -arena->height / 2 - height / 2});

  // Enable it
  SetCharacterActive(true);
}

void FallOffDeath::SetCharacterActive(bool active)
{
  gameObject.RequireComponent<SpriteRenderer>()->SetEnabled(active);
  gameObject.RequireComponent<Animator>()->SetEnabled(active);
  gameObject.RequireComponent<Rigidbody>()->SetEnabled(active);
  gameObject.RequireComponent<Collider>()->SetEnabled(active);
  gameObject.RequireComponent<Movement>()->SetEnabled(active);
  gameObject.RequireComponent<PlayerInput>()->SetEnabled(active);
  gameObject.RequireComponent<CharacterStateManager>()->SetEnabled(active);
  gameObject.RequireComponent<CharacterController>()->SetEnabled(active);
}

bool FallOffDeath::IsDead() const { return dead; }
