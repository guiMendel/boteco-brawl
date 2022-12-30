#ifndef __PHYSICS_LAYER__
#define __PHYSICS_LAYER__

// Don't mind this!
#define PRINTABLE(enumValue) translation[PhysicsLayer::enumValue] = #enumValue;

// When defined, allows for printing the collision matrix on game state construction
#define PRINT_COLLISION_MATRIX

// Allow for specifying which groups of objects should have collision
// "None" layer will inherit it's closest parent's layer
enum class PhysicsLayer
{
  Default,
  Character,
  Scenario,
  Hazard,
  None
};

// Necessary for each enum value in order to print layer names
#define ADD_LAYER_TRANSLATION \
  PRINTABLE(Character)    \
  PRINTABLE(Scenario)     \
  PRINTABLE(Hazard)       \
  PRINTABLE(Default)

#endif