#ifndef __PHYSICS_LAYER__
#define __PHYSICS_LAYER__

// Allows for printing the collision matrix on game state construction
#define PRINT_COLLISION_MATRIX

// Allow for specifying which groups of objects should have collision
enum class PhysicsLayer
{
  Character,
  Scenario,
  Hazard,
  Default
};

#endif