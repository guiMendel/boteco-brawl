#include "Debug.h"
#include "Game.h"
#include "Camera.h"
#include <SDL.h>

void Debug::DrawPoint(Vector2 point, bool convertToScreen)
{
  if (convertToScreen)
  {
    point = Camera::GetMain()->WorldToScreen(point);
  }

  auto renderer = Game::GetInstance().GetRenderer();

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

  SDL_RenderDrawPoint(renderer, point.x, point.y);
}

// Got this from the internet
void Debug::DrawCircle(Vector2 center, float radius, bool convertToScreen)
{
  auto renderer = Game::GetInstance().GetRenderer();

  radius = Camera::GetMain()->GetPixelsPerUnit() * radius;

  if (convertToScreen)
    center = Camera::GetMain()->WorldToScreen(center);

  int32_t centreX = center.x;
  int32_t centreY = center.y;

  const int32_t diameter = (radius * 2);

  int32_t x = (radius - 1);
  int32_t y = 0;
  int32_t tx = 1;
  int32_t ty = 1;
  int32_t error = (tx - diameter);

  while (x >= y)
  {
    //  Each of the following renders an octant of the circle
    SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
    SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
    SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
    SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
    SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
    SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
    SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
    SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

    if (error <= 0)
    {
      ++y;
      error += ty;
      ty += 2;
    }

    if (error > 0)
    {
      --x;
      tx += 2;
      error += (tx - diameter);
    }
  }
}