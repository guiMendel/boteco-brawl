#ifndef __MOUSE_CURSOR__
#define __MOUSE_CURSOR__

#include <memory>
#include <string>
#include "Helper.h"
#include "Vector2.h"

struct MouseCursor
{
  MouseCursor(std::string imagePath, Vector2 offset = Vector2::Zero());

  ~MouseCursor();

  // The cursor's surface image
  Helper::auto_unique_ptr<SDL_Surface> surface;

  // The cursor's SDL cursor struct image
  Helper::auto_unique_ptr<SDL_Cursor> sdlCursor;

  // Activates this cursor
  void Activate() const;

  // Whether this is the active cursor
  bool IsActive() const;
};

#endif