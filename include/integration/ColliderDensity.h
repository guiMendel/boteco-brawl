#ifndef __COLLIDER_DENSITY__
#define __COLLIDER_DENSITY__

// Specifies possible densities for each type of collider
// All labels MUST contain a value!
enum class ColliderDensity
{
  Default = 10,
  Wood = 10,
  Metal = 50,
  Character = 5,
  Ground = 25
};

#endif