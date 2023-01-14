#ifndef __DAMAGE__
#define __DAMAGE__

#include "Vector2.h"
#include <memory>

class GameObject;

struct Damage
{
  // How much heat to add
  float heatDamage;

  // How much impulse to apply, along with direction
  Vector2 impulse;

  // How much time to leave target stunned, unable to move
  float stunTime;

  // Reference to object dealing this damage
  std::weak_ptr<GameObject> weakAuthor;
};

#endif