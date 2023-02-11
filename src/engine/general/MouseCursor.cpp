#include "MouseCursor.h"
#include <SDL_image.h>

MouseCursor::MouseCursor(std::string imagePath, Vector2 offset)
    : surface(IMG_Load(imagePath.c_str()), SDL_FreeSurface),
      sdlCursor(SDL_CreateColorCursor(surface.get(), int(offset.x), int(offset.y)), SDL_FreeCursor) {}

MouseCursor::~MouseCursor()
{
  // If this is the active cursor, switch back to default cursor
  if (IsActive())
    SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW));
}

void MouseCursor::Activate() const { SDL_SetCursor(sdlCursor.get()); }

bool MouseCursor::IsActive() const { return SDL_GetCursor() == sdlCursor.get(); }
