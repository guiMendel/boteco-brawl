#include "UIImage.h"
#include "Resources.h"

using namespace std;

UIImage::UIImage(Canvas &canvas, string name, shared_ptr<UIContainer> parent, string imagePath)
    : UIContent(canvas, name, parent) { SetImagePath(imagePath); }

void UIImage::SetImagePath(string imagePath)
{
  this->imagePath = imagePath;
  ReloadTextureDimensions();
}

string UIImage::GetImagePath() { return imagePath; }

void UIImage::UseTexture(function<void(SDL_Texture *)> procedure)
{
  // Get image texture
  auto texture = Resources::GetTexture(imagePath);

  // Get color
  auto modulateColor = style->imageColor.Get();

  // Apply color modulation
  if (modulateColor != Color::White())
  {
    SDL_SetTextureColorMod(texture.get(), modulateColor.red, modulateColor.green, modulateColor.blue);

    if (modulateColor.alpha < 255)
      SDL_SetTextureAlphaMod(texture.get(), modulateColor.alpha);
  }

  // Execute procedure
  procedure(texture.get());

  // Revert modifications to texture
  SDL_SetTextureColorMod(texture.get(), 255, 255, 255);
  SDL_SetTextureAlphaMod(texture.get(), 255);
}

void UIImage::Render()
{
  // Get target dimensions
  size_t targetWidth = width.AsRealPixels();
  size_t targetHeight = height.AsRealPixels();

  auto camera = Camera::GetMain();
  auto pixelPosition = canvas.CanvasToScreen(GetContentPosition());

  SDL_Rect destinationRect = {int(pixelPosition.x), int(pixelPosition.y), int(targetWidth), int(targetHeight)};
  SDL_Rect sourceRect = {0, 0, textureWidth, textureHeight};

  // Puts the texture in the renderer
  auto render = [&](SDL_Texture *texture)
  {
    SDL_RenderCopy(
        Game::GetInstance().GetRenderer(),
        texture,
        &sourceRect,
        &destinationRect);
  };

  // Apply render procedure
  UseTexture(render);

  // Debug render
  UIObject::Render();
}

void UIImage::ReloadTextureDimensions()
{
  // Get the texture
  auto texture = Resources::GetTexture(imagePath);

  // Query it's dimensions
  SDL_QueryTexture(texture.get(), nullptr, nullptr, &textureWidth, &textureHeight);
}

size_t UIImage::GetContentRealPixelsAlong(UIDimension::Axis axis)
{
  return (axis == UIDimension::Horizontal ? textureWidth : textureHeight) * style->imageScaling.Get();
}
