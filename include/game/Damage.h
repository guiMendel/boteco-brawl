#ifndef __DAMAGE__
#define __DAMAGE__

#include "Vector2.h"
#include <memory>

class GameObject;

struct DamageParameters;

// A struct that already derived impulse direction and heat damage from the attacker's parameters
struct Damage
{
  friend struct DamageParameters;
  
  // How much heat to add
  float heatDamage;

  // How much impulse to apply, along with direction
  // Corresponds to actual direction to which target will be launched
  Vector2 impulse;

  // How much time to leave target stunned, unable to move
  float stunTime;

  // Reference to object dealing this damage
  std::weak_ptr<GameObject> weakAuthor;

private:
  // Make constructor private
  Damage() = default;
};

// A struct that contains information on how to derive a Damage struct
struct DamageParameters
{
  // Heat damage multiplier
  float heatMultiplier;

  // How much impulse to apply, along with direction
  // Right direction means aligned to facing direction
  Vector2 impulse;

  // How much time to leave target stunned, unable to move
  float stunTime;

  // Derive a damage struct
  Damage DeriveDamage(float baseHeatDamage, std::shared_ptr<GameObject> author) const;
};

#endif