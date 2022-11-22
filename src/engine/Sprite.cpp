#include "Sprite.h"
#include "Resources.h"
#include "Game.h"
#include "Camera.h"
#include <string>

using namespace std;
using namespace Helper;

Sprite::Sprite(GameObject &associatedObject, RenderLayer renderLayer, float pixelsPerUnit, int renderOrder, bool centerObject)
    : Component(associatedObject), centered(centerObject), pixelsPerUnit(pixelsPerUnit), renderLayer(renderLayer), renderOrder(renderOrder)
{
}

// Constructor with image file name
Sprite::Sprite(GameObject &associatedObject, const std::string fileName, RenderLayer renderLayer, float pixelsPerUnit, int renderOrder, bool centerObject) : Sprite(associatedObject, renderLayer, pixelsPerUnit, renderOrder, centerObject)
{
  Load(fileName);
}

void Sprite::Load(const string fileName)
{
  // Get texture from resource manager
  texture = Resources::GetTexture(fileName);

  int width, height;

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

  // Get the real position box
  SDL_Rect destinationRect = (SDL_Rect)Camera::GetMain()->WorldToScreen(
      Rectangle(position, GetWidth(), GetHeight()));

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

void Sprite::SetTargetDimension(int width, int height)
{
  targetWidth = width;
  targetHeight = height;
}

float Sprite::GetWidth() const
{
  if (targetWidth >= 0)
    return targetWidth * gameObject.localScale.x;

  // If target height is also -1, use clip width (convert pixels to game units using own proportion)
  if (targetHeight < 0)
    return clipRect.w * gameObject.localScale.x / pixelsPerUnit;

  // Otherwise, use aspect ratio
  return targetHeight * clipRect.w / clipRect.h * gameObject.localScale.x;
}

float Sprite::GetHeight() const
{
  if (targetHeight >= 0)
    return targetHeight * gameObject.localScale.y;

  // If target width is also -1, use clip Height (convert pixels to game units using own proportion)
  if (targetWidth < 0)
  {
    return clipRect.h * gameObject.localScale.y / pixelsPerUnit;
  }

  // Otherwise, use aspect ratio
  return targetWidth * clipRect.h / clipRect.w * gameObject.localScale.y;
}
