#include "Recipes.h"
#include "Alien.h"
#include "Sprite.h"
#include "InputManager.h"
#include "Game.h"
#include "Minion.h"
#include "Debug.h"
#include "MainState.h"
#include <iostream>

using namespace std;

// Rotation speed, in radians
const float Alien::rotationSpeed{0.2f};

// Movement acceleration
const float Alien::acceleration{1.5f};
const float Alien::maxSpeed{150.0f};

// Total minion count
const Vector2 Alien::totalMinions{2, 7};

// Total health points
const float Alien::healthPoints{500.0f};

const Vector2 Alien::idleTime{0.5f, 4.0f};

// Helper functions
shared_ptr<GameObject> NearestMinion(vector<weak_ptr<GameObject>> &minions, Vector2 position);

Alien::Alien(GameObject &associatedObject) : Component(associatedObject) {}

void Alien::OnBeforeDestroy()
{
  auto ExplosionRecipe = Recipes::OneShotAnimation("./assets/image/aliendeath.png", Vector2(127.25f, 133), 0.4f);

  gameState.CreateObject("Alien Explosion", ExplosionRecipe, gameObject.GetPosition());
}

void Alien::Start()
{
  movementWeak = gameObject.RequireComponent<Movement>();
  penguinWeak = gameState.FindObjectOfType<PenguinBody>();

  // Get game state reference
  auto &gameState = Game::GetInstance().GetState();

  // Add minions
  int minionCount = RandomRange(totalMinions.x, totalMinions.y);

  for (int i = 0; i < minionCount; i++)
  {
    gameState.CreateObject(
        "Minion",
        Recipes::Minion(
            dynamic_pointer_cast<Alien>(GetShared()), 2 * M_PI * i / minionCount));
  }

  // Start timer
  gameObject.timer.Reset("idle", -RandomRange(idleTime.x, idleTime.y));
}

void Alien::Update([[maybe_unused]] float deltaTime)
{
  // If idle, check if timer is up
  if (state == State::idle)
  {
    // If it's up, move
    if (gameObject.timer.Get("idle") >= 0)
      Chase();
  }

  // If it's chasing, do nothing

  // Rotate slowly
  gameObject.localRotation += rotationSpeed * deltaTime;
}

void Alien::Chase()
{
  // Ignore if no penguin
  if (penguinWeak.expired())
    return;

  // Switch state
  state = State::moving;

  // Get movement component
  LOCK(movementWeak, movement);

  // Get penguin
  LOCK(penguinWeak, penguin);

  // Move towards player
  movement->MoveTo(
      penguin->gameObject.GetPosition(), [this]()
      { Arrive(); });
}

void Alien::Arrive()
{
  // Switch to idle state
  state = State::idle;

  // Get penguin
  if (penguinWeak.expired() == false)
  {
    LOCK(penguinWeak, penguin);

    // Shoot at player
    Shoot(penguin->gameObject.GetPosition());
  }

  // Start timer
  gameObject.timer.Reset("idle", -RandomRange(idleTime.x, idleTime.y));
}

void Alien::Shoot(Vector2 position)
{
  // Ignore if no minions
  if (minions.empty())
    return;

  // Pick nearest minion
  auto minion = NearestMinion(minions, position);

  // Check that it's valid
  if (!minion)
    return;

  // Tell it to shoot at the target
  minion->GetComponent<Minion>()->Shoot(position);
}

shared_ptr<GameObject> NearestMinion(vector<weak_ptr<GameObject>> &minions, Vector2 position)
{
  // Best distance so far
  float bestDistance = -1;

  shared_ptr<GameObject> bestMinion;

  // For each minion
  auto minionIterator = minions.begin();

  while (minionIterator != minions.end())
  {
    // Try to lock it
    if (auto minion = minionIterator->lock())
    {
      // Get it's distance
      float distance = Vector2::Distance(minion->GetPosition(), position);

      // If it's better
      if (bestDistance == -1 || distance < bestDistance)
      {
        bestDistance = distance;
        bestMinion = minion;
      }

      minionIterator++;
    }
    // If failed, it's gone, so forget it
    else
      minionIterator = minions.erase(minionIterator);
  }

  return bestMinion;
}
