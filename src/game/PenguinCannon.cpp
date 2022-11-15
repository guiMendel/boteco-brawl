#include "Recipes.h"
#include "PenguinCannon.h"
#include "InputManager.h"
#include "SpriteAnimator.h"
#include "Projectile.h"
#include "MainState.h"

using namespace std;

// Projectile speed
const float PenguinCannon::projectileSpeed{300};

// Projectile time to live, in seconds
const float PenguinCannon::projectileTimeToLive{5};

// Damage of projectile
const float PenguinCannon::projectileDamage{50};

// Cooldown
const float PenguinCannon::cooldown{0.3f};

void PenguinCannon::Start()
{
  // Start a cooldown timer
  gameObject.timer.Reset("cooldown", cooldown);
}

void PenguinCannon::Update([[maybe_unused]] float deltaTime)
{
  auto &input = InputManager::GetInstance();

  // Point at mouse
  gameObject.SetRotation(
      Vector2::AngleBetween(gameObject.GetPosition(), input.GetMouseWorldCoordinates()));

  // Shoot on mouse down
  if (input.IsMouseDown(RIGHT_MOUSE_BUTTON))
    Shoot();
}

void PenguinCannon::Shoot()
{
  // Check if cooldown is done
  if (gameObject.timer.Get("cooldown") < cooldown)
    return;

  // Restart cooldown
  gameObject.timer.Reset("cooldown");

  // Create the projectile
  gameState
      .CreateObject(
          "Penguin Projectile",
          Recipes::Projectile(
              "./assets/image/penguinbullet.png", Vector2(30, 29), 0.2f, false,
              Tag::Enemy, gameObject.GetRotation(), projectileSpeed, projectileTimeToLive, projectileDamage),
          GunPointPosition());
}

Vector2 PenguinCannon::GunPointPosition()
{
  // Get image size
  int width = gameObject.GetComponent<Sprite>()->GetWidth();

  // Gun point is on end of image
  return gameObject.GetPosition() + Vector2::Angled(gameObject.GetRotation(), width / 2);
}
