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
  if (IsEnabled() == false)
    return;

  // Get target dimensions
  int targetWidth = GetUnpaddedWidth() * GetScale().x;
  int targetHeight = GetUnpaddedHeight() * GetScale().y;

  auto camera = Camera::GetMain();
  // auto pixelPosition = canvas.CanvasToScreen(GetContentPosition()) -
  auto pixelPosition = canvas.CanvasToScreen(GetContentPosition()) -
                       Vector2{float(GetUnpaddedWidth()), float(GetUnpaddedHeight())} * (GetScale() - Vector2::One()) / 2;

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

int UIImage::GetContentRealPixelsAlong(UIDimension::Axis axis, UIDimension::Calculation)
{
  return (axis == UIDimension::Horizontal ? textureWidth : textureHeight) * style->imageScaling.Get();
}

void UIImage::SetSizePreserveRatio(UIDimension::Axis mainAxis, UIDimension::UnitType unit, float value)
{
  // Get both dimensions
  auto &mainDimension = GetDimension(mainAxis);
  auto &crossDimension = GetDimension(UIDimension::GetCrossAxis(mainAxis));

  // Get current ratio
  float ratio = float(crossDimension.AsRealPixels()) / float(mainDimension.AsRealPixels());

  // Set main dimension to new value
  mainDimension.Set(unit, value);

  // Set cross dimension respectively
  crossDimension.Set(UIDimension::RealPixels, float(mainDimension.AsRealPixels()) * ratio);
}

float UIImage::GetScaling(UIDimension::Axis axis)
{
  return float(GetRealPixelsAlong(axis, false)) /
         float(axis == UIDimension::Horizontal ? textureWidth : textureHeight);
}
