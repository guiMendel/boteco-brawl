#include "SpriteRenderer.h"
#include "Resources.h"
#include "Game.h"
#include "Camera.h"
#include <string>

using namespace std;
using namespace Helper;

// Aspect ratio of game screen
static const float screenRatio = float(Game::screenWidth) / float(Game::screenHeight);

SpriteRenderer::SpriteRenderer(GameObject &associatedObject, RenderLayer renderLayer, int renderOrder)
    : WorldComponent(associatedObject), renderLayer(renderLayer), renderOrder(renderOrder), parallaxReferenceCameraSize(Camera::initialSize)
{
}

// Constructor with image file name
SpriteRenderer::SpriteRenderer(GameObject &associatedObject, const string fileName, RenderLayer renderLayer, int renderOrder) : SpriteRenderer(associatedObject, renderLayer, renderOrder)
{
  sprite = Resources::GetSprite(fileName);
}

void SpriteRenderer::Render() { Render(worldObject.GetPosition()); }

Vector2 SpriteRenderer::RenderPositionFor(Vector2 position, shared_ptr<Sprite> referenceSprite) const
{
  // Apply offset
  position += offset * worldObject.GetScale();

  // Apply parallax
  position = ApplyParallax(position);

  referenceSprite = referenceSprite == nullptr ? this->sprite : referenceSprite;

  Assert(referenceSprite != nullptr, "Sprite renderer had no sprite set");

  // Get sprite's dimensions
  auto [width, height] = GetSpriteDimensionsParallax(referenceSprite);

  return position - Vector2(width, height) * anchorPoint;
}

void SpriteRenderer::Render(Vector2 position)
{
  // Get parallaxed position
  position = RenderPositionFor(position);

  // Get parallaxed dimensions
  auto [width, height] = GetSpriteDimensionsParallax();

  auto camera = Camera::GetMain();
  auto pixelPosition = camera->WorldToScreen(position);

  SDL_Rect destinationRect = {
      int(pixelPosition.x), int(pixelPosition.y),
      int(width * camera->GetRealPixelsPerUnit()), int(height * camera->GetRealPixelsPerUnit())};

  // Detect flips
  SDL_RendererFlip horizontalFlip = worldObject.localScale.x < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
  SDL_RendererFlip verticalFlip = worldObject.localScale.y < 0 ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;

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
        Helper::RadiansToDegrees(worldObject.GetRotation()),
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

pair<float, float> SpriteRenderer::GetSpriteDimensionsParallax(shared_ptr<Sprite> referenceSprite) const
{
  // Coalesce sprite
  if (referenceSprite == nullptr)
    referenceSprite = GetSprite();

  // Get scale
  auto scale = worldObject.GetScale().GetAbsolute();

  // Get sprite's original dimensions scaled
  auto [width, height] = make_pair(referenceSprite->GetWidth(scale.x), referenceSprite->GetHeight(scale.y));

  // Catch override
  if (overrideWidthPixels > 0)
  {
    height = overrideWidthPixels * (height / width) * Camera::GetMain()->GetUnitsPerRealPixel();
    width = overrideWidthPixels * Camera::GetMain()->GetUnitsPerRealPixel();

    return {width, height};
  }

  if (parallax == 0)
    return {width, height};

  // Get reference camera's dimensions
  float referenceHeight = parallaxReferenceCameraSize * 2;
  float referenceWidth = screenRatio * referenceHeight;

  // Get dimensions proportional size to reference camera size
  auto proportionalWidth = width / referenceWidth;
  auto proportionalHeight = height / referenceHeight;

  // Get current camera's dimensions
  float cameraHeight = Camera::GetMain()->GetSize() * 2;
  float cameraWidth = screenRatio * cameraHeight;

  return make_pair(
      Lerp(width, cameraWidth * proportionalWidth, parallax),
      Lerp(height, cameraHeight * proportionalHeight, parallax));
}

void SpriteRenderer::SetParallax(float parallax, float referenceCameraSize)
{
  this->parallax = parallax;
  parallaxReferenceCameraSize = referenceCameraSize;
}

Vector2 SpriteRenderer::ApplyParallax(Vector2 position) const
{
  if (parallax == 0)
    return position;

  // Get camera
  auto camera = Camera::GetMain();

  // Get what this position would be if the camera were at origin
  Vector2 staticScreenPosition = camera->WorldToScreen(position, true);

  return Lerp(
      position,
      camera->ScreenToWorld(staticScreenPosition),
      parallax);
}
