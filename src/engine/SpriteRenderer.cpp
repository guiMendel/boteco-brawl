#include "SpriteRenderer.h"
#include "Resources.h"
#include "Game.h"
#include "Camera.h"
#include <string>

using namespace std;
using namespace Helper;

SpriteRenderer::SpriteRenderer(GameObject &associatedObject, RenderLayer renderLayer, int renderOrder)
    : Component(associatedObject), renderLayer(renderLayer), renderOrder(renderOrder)
{
}

// Constructor with image file name
SpriteRenderer::SpriteRenderer(GameObject &associatedObject, const string fileName, RenderLayer renderLayer, int renderOrder) : SpriteRenderer(associatedObject, renderLayer, renderOrder)
{
  sprite = Resources::GetSprite(fileName);
}

void SpriteRenderer::Render() { Render(gameObject.GetPosition()); }

Vector2 SpriteRenderer::RenderPositionFor(Vector2 position, shared_ptr<Sprite> referenceSprite) const
{
  // Apply offset
  position += offset * gameObject.GetScale();

  auto scale = gameObject.GetScale().GetAbsolute();
  auto sprite = referenceSprite == nullptr ? this->sprite : referenceSprite;

  Assert(sprite != nullptr, "Sprite renderer had no sprite set");

  // Get sprite's dimensions
  auto [width, height] = make_pair(sprite->GetWidth(scale.x), sprite->GetHeight(scale.y));

  // Check for override
  if (overrideWidthPixels > 0)
  {
    height = overrideWidthPixels * (height / width) / Camera::GetMain()->GetRealPixelsPerUnit();
    width = overrideWidthPixels / Camera::GetMain()->GetRealPixelsPerUnit();
  }

  return position - Vector2(width, height) * anchorPoint;
}

void SpriteRenderer::Render(Vector2 position)
{
  position = RenderPositionFor(position);

  auto scale = gameObject.GetScale().GetAbsolute();

  auto [width, height] = make_pair(sprite->GetWidth(scale.x), sprite->GetHeight(scale.y));

  auto camera = Camera::GetMain();
  auto pixelPosition = camera->WorldToScreen(position);

  SDL_Rect destinationRect = {
      int(pixelPosition.x), int(pixelPosition.y),
      int(width * camera->GetRealPixelsPerUnit()), int(height * camera->GetRealPixelsPerUnit())};

  // Check for width override
  if (overrideWidthPixels > 0)
  {
    destinationRect.w = overrideWidthPixels;
    destinationRect.h = overrideWidthPixels * height / width;
  }

  // Detect flips
  SDL_RendererFlip horizontalFlip = gameObject.localScale.x < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
  SDL_RendererFlip verticalFlip = gameObject.localScale.y < 0 ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;

  // Get source clip
  auto sourceRect = sprite->GetClip();

  // Puts the texture in the renderer
  auto render = [&](SDL_Texture *texture)
  {
    SDL_RenderCopyEx(
        Game::GetInstance().GetRenderer(),
        texture,
        &sourceRect,
        &destinationRect,
        Helper::RadiansToDegrees(gameObject.GetRotation()),
        nullptr,
        SDL_RendererFlip(horizontalFlip | verticalFlip));
  };

  // Apply render procedure
  UseTexture(render);
}

Vector2 SpriteRenderer::GetOffset() const { return offset; }

void SpriteRenderer::SetOffset(Vector2 newOffset)
{
  offset = newOffset;
  OnSetOffset.Invoke(offset);
}

void SpriteRenderer::UseTexture(function<void(SDL_Texture *)> procedure)
{
  // Get texture from sprite
  auto texture = sprite->GetTexture();

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

void SpriteRenderer::SetColor(Color modulateColor, Color addColor)
{
  if (modulateColor.IsValid())
    this->modulateColor = modulateColor;

  if (addColor.IsValid())
    this->addColor = addColor;

  // Reset generated texture
  lastTexture = nullptr;
}

pair<Color, Color> SpriteRenderer::GetColors() const { return {modulateColor, addColor}; }

void SpriteRenderer::SetSprite(shared_ptr<Sprite> newSprite)
{
  sprite = newSprite;

  // Reset generated texture
  lastTexture = nullptr;
}

shared_ptr<Sprite> SpriteRenderer::GetSprite() const { return sprite; }

void SpriteRenderer::OverrideWidthPixels(int newWidth) { overrideWidthPixels = newWidth; }

void SpriteRenderer::SetAnchorPoint(Vector2 point) { anchorPoint = point; }
