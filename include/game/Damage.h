#ifndef __DAMAGE__
#define __DAMAGE__

#include "Vector2.h"

struct Damage
{
  // How much heat to add
  float heatDamage;

  // How much impulse to apply, along with direction
  Vector2 impulse;
};

#endif