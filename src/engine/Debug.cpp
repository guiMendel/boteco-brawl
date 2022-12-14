#include "Debug.h"
#include "Game.h"
#include "Camera.h"
#include <SDL.h>

void Debug::DrawPoint(Vector2 point, Color color)
{
  point = Camera::GetMain()->WorldToScreen(point);

  auto renderer = Game::GetInstance().GetRenderer();

  SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, color.alpha);

  SDL_RenderDrawPoint(renderer, point.x, point.y);
}

// Got this from the internet
void Debug::DrawCircle(Circle circle, Color color)
{
  auto renderer = Game::GetInstance().GetRenderer();

  auto radius = Camera::GetMain()->GetRealPixelsPerUnit() * circle.radius;

  auto center = Camera::GetMain()->WorldToScreen(circle.center);

  int32_t centreX = center.x;
  int32_t centreY = center.y;

  const int32_t diameter = (radius * 2);

  int32_t x = (radius - 1);
  int32_t y = 0;
  int32_t tx = 1;
  int32_t ty = 1;
  int32_t error = (tx - diameter);

  SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, color.alpha);

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

void Debug::DrawBox(Rectangle box, Color color)
{
  auto camera = Camera::GetMain();

  // Create an SDL point for each vertex
  SDL_Point vertices[5];

  // Starting and final points are top left
  vertices[0] = (SDL_Point)camera->WorldToScreen(box.TopLeft());
  vertices[1] = (SDL_Point)camera->WorldToScreen(box.BottomLeft());
  vertices[2] = (SDL_Point)camera->WorldToScreen(box.BottomRight());
  vertices[3] = (SDL_Point)camera->WorldToScreen(box.TopRight());
  vertices[4] = (SDL_Point)camera->WorldToScreen(box.TopLeft());

  // Get renderer
  auto renderer = Game::GetInstance().GetRenderer();

  // Set paint color to green
  SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, color.alpha);

  // Paint collider edges
  SDL_RenderDrawLines(renderer, vertices, 5);
}
