#include "Sprite.h"
#include "Resources.h"
#include "Game.h"
#include "Camera.h"
#include <string>

using namespace std;
using namespace Helper;

void Sprite::Load(const string fileName)
{
  // Get texture from resource manager
  texture = Resources::GetTexture(fileName);

  // Get it's dimensions
  SDL_QueryTexture(texture.get(), nullptr, nullptr, &width, &height);

  // Set the clip to the full image
  SetClip(0, 0, width, height);
}

void Sprite::SetClip(int x, int y, int width, int height)
{
  clipRect = {x, y, width, height};
}

void Sprite::Render(Vector2 position)
{
  // Offset coordinates if centered
  if (centered)
  {
    position -= Vector2(GetWidth() / 2, GetHeight() / 2);
  }

  // Get the real position
  Vector2 offsetPosition = Camera::GetInstance().WorldToScreen(position);

  // Get destination rectangle
  SDL_Rect destinationRect{
      (int)offsetPosition.x, (int)offsetPosition.y, GetWidth(), GetHeight()};

  // Put the texture in the renderer
  SDL_RenderCopyEx(
      Game::GetInstance().GetRenderer(),
      texture.get(),
      &clipRect,
      &destinationRect,
      Helper::RadiansToDegrees(gameObject.GetRotation()),
      nullptr,
      SDL_FLIP_NONE);
}
