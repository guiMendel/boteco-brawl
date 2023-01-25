#ifndef __FALL_OFF_DEATH__
#define __FALL_OFF_DEATH__

#include "GameObject.h"
#include "Component.h"
#include "Arena.h"

class FallDeath : public Component
{
public:
  // How far from the screen edge the character dies
  static const float deathMargin;

  // Seconds a character takes to respawn
  static const float respawnDelay;

  // How many lives a character starts off with
  static const int startingLives;

  // Triggered on falling off of stage
  Event OnFall;

  // Triggered when all lives are spent
  Event OnDeath;

  FallDeath(GameObject &associatedObject);
  virtual ~FallDeath() {}

  void Update(float) override;

  // Disable or enable the character (remove it from arena)
  void SetCharacterActive(bool active);

  // Triggers a fall animation, and start respawn timer
  void Fall();

  // Respawns character at the top of the screen
  void Respawn();

  // Whether character is currently fallen (regardless of whether it's also dead)
  bool IsFallen() const;

  // Whether character is dead
  bool IsDead() const;

  // Whether character is fallen and not dead
  bool IsRespawning() const;

  // How many lives this character still has
  int GetLives() const;

  // How many seconds has elapsed since character fell off (or since it's creation if it hasn't yet)
  float GetLastFallAge() const;

private:
  // Play death effect
  void PlayEffect() const;

  // Arena reference
  std::weak_ptr<Arena> weakArena;

  // Time of last fall
  float lastFallTime{float(SDL_GetTicks())};

  // Lives still left
  int lives{startingLives};

  bool fallen{false};
};

#endif