#ifndef __FALL_OFF_DEATH__
#define __FALL_OFF_DEATH__

#include "GameObject.h"
#include "Component.h"
#include "Arena.h"

class FallOffDeath : public Component
{
public:
  // How far from the screen edge the character dies
  static const float deathMargin;

  // Seconds a character takes to respawn
  static const float respawnDelay;

  FallOffDeath(GameObject &associatedObject);
  virtual ~FallOffDeath() {}

  void Update(float);

  // Disable or enable the character (remove it from arena)
  void SetCharacterActive(bool active);

  // Triggers a death animation, and start respawn timer
  void Die();

  // Respawns character at the top of the screen
  void Respawn();

private:
  // Arena reference
  std::weak_ptr<Arena> weakArena;
};

#endif