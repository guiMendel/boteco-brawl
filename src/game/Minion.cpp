#include "Recipes.h"
#include "Minion.h"
#include "Projectile.h"
#include "Game.h"
#include "Sprite.h"
#include "SpriteAnimator.h"
#include "MainState.h"
#include <math.h>

using namespace std;

// Speed at which to orbit the host alien, in radians
const float Minion::angularSpeed{0.5f};

// Speed at which the radius changes
const float Minion::radiusFloatSpeed{1};

// Limits to radius floating
const float Minion::radiusLimits[2]{90, 160};

// Limits to scale
const float Minion::scaleLimits[2]{0.75f, 1.25f};

// Projectile speed
const float Minion::projectileSpeed{300};

// Projectile time to live, in seconds
const float Minion::projectileTimeToLive{5};

// Damage of projectile
const float Minion::projectileDamage{35};

const float Minion::healthPoints{100.0f};

Minion::Minion(GameObject &associatedObject, weak_ptr<GameObject> hostPointer, float startingArc)
    : Component(associatedObject), hostPointer(hostPointer), arc(startingArc)
{
  // Initialize radius
  orbitRadius = (rand() % (int)floor(radiusLimits[1] - radiusLimits[0])) + radiusLimits[0];

  // Set scale
  auto scale = RandomRange(scaleLimits[0], scaleLimits[1]);
  gameObject.localScale = {scale, scale};
}

void Minion::OnBeforeDestroy()
{
  auto ExplosionRecipe = Recipes::OneShotAnimation("./assets/image/miniondeath.png", Vector2(63.75f, 67), 0.25f);

  gameState.CreateObject("Minion Explosion", ExplosionRecipe, gameObject.GetPosition());
}

void Minion::Update(float deltaTime)
{
  auto hostLocked = hostPointer.lock();

  // If host is gone, destroy self
  if (!hostLocked)
  {
    gameObject.RequestDestroy();
    return;
  }

  // Update orbit position
  arc += angularSpeed * deltaTime;

  // Update radius position
  orbitRadius += radiusFloatSpeed * floatDirection;

  // Add rotation so that bottom part of sprite faces host
  gameObject.localRotation = -(Vector2::AngleBetween(gameObject.GetPosition(), hostLocked->GetPosition()) - M_PI / 2);

  if (orbitRadius <= radiusLimits[0])
  {
    orbitRadius = radiusLimits[0];
    floatDirection = 1;
  }

  else if (orbitRadius >= radiusLimits[1])
  {
    orbitRadius = radiusLimits[1];
    floatDirection = -1;
  }

  // Get radial position vector
  Vector2 radialPosition = Vector2::Right(orbitRadius).Rotated(arc);

  // Update position
  gameObject.SetPosition(hostLocked->GetPosition() + radialPosition);
}

void Minion::Shoot(Vector2 target)
{
  // Get angle to target
  float targetAngle = Vector2::AngleBetween(gameObject.GetPosition(), target);

  // Create the projectile
  gameState
      .CreateObject(
          "Minion Projectile",
          Recipes::Projectile(
              "./assets/image/minionbullet2.png", Vector2(33, 12), 0.2f, true,
              Tag::Player, targetAngle, projectileSpeed, projectileTimeToLive, projectileDamage),
          gameObject.GetPosition());
}