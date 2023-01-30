#ifndef __CANVAS__
#define __CANVAS__

#include "WorldComponent.h"
#include "UIRoot.h"

// Creates a space onto which UI can be inserted
class Canvas : public WorldComponent
{
public:
  // Different types of canvas
  enum class Space
  {
    // Takes up the whole space of the screen, and is completely disconnected from the game world
    Global,

    // Has a position in the game world, but it's size is independent of it and is given in real pixels
    WorldFixedSize,

    // Is positioned in the game world and has a size in world units
    World
  };

  // Explicitly initialize shape
  Canvas(GameObject &associatedObject, Space space);

  virtual ~Canvas() {}

  // How the canvas is related to the world
  Space space;

  // Size of the canvas
  // Ignored when Global space, in real pixels when WorldFixedSize space, and in world units when World space
  Vector2 size;

  // The root UI Object
  UIRoot root;
};

#endif