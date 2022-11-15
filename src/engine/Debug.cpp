#include "Debug.h"
#include "Game.h"
#include "Camera.h"
#include <SDL.h>

void Debug::DrawPoint(Vector2 point, bool convertToScreen)
{
  if (convertToScreen)
  {
    point = Camera::GetInstance().WorldToScreen(point);
  }

  auto renderer = Game::GetInstance().GetRenderer();

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

  SDL_RenderDrawPoint(renderer, point.x, point.y);
}