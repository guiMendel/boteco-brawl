#include "Sprite.h"
#include "Resources.h"

using namespace std;

Sprite::Sprite(const string initialTexture, SpriteConfig config) : config(config)
{
  Load(initialTexture);
}

void Sprite::Load(const string fileName)
{
  // Get texture from resource manager
  texture = Resources::GetTexture(fileName);

  // Get it's dimensions
  SDL_QueryTexture(texture.get(), nullptr, nullptr, &width, &height);

  // Set the clip to the full image
  SetClip(SDL_Rect{0, 0, width, height});
}

SDL_Rect Sprite::GetClip() const { return clipRect; }

void Sprite::SetClip(SDL_Rect rect)
{
  // Sanitize dimensions and allow automatic size clipping
  if (rect.w < 0)
    rect.w = width - rect.x;
  if (rect.h < 0)
    rect.h = height - rect.y;

  clipRect = rect;
}

void Sprite::SetTargetDimension(int width, int height)
{
  config.targetWidth = width;
  config.targetHeight = height;
}

float Sprite::GetWidth(float scale) const
{
  if (config.targetWidth >= 0)
    return config.targetWidth * scale;

  // If target height is also -1, use clip width (convert pixels to game units using own proportion)
  if (config.targetHeight < 0)
    return clipRect.w * scale / config.pixelsPerUnit;

  // Otherwise, use aspect ratio
  return config.targetHeight * clipRect.w / clipRect.h * scale;
}

float Sprite::GetHeight(float scale) const
{
  if (config.targetHeight >= 0)
    return config.targetHeight * scale;

  // If target width is also -1, use clip Height (convert pixels to game units using own proportion)
  if (config.targetWidth < 0)
  {
    return clipRect.h * scale / config.pixelsPerUnit;
  }

  // Otherwise, use aspect ratio
  return config.targetWidth * clipRect.h / clipRect.w * scale;
}

void Sprite::SetConfig(SpriteConfig newConfig) { config = newConfig; }

std::shared_ptr<SDL_Texture> Sprite::GetTexture() const { return texture; }
