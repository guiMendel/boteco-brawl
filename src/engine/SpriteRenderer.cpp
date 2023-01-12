#include "SpriteRenderer.h"
#include "Resources.h"
#include "Game.h"
#include "Camera.h"
#include <string>

using namespace std;
using namespace Helper;

SpriteRenderer::SpriteRenderer(GameObject &associatedObject, RenderLayer renderLayer, int renderOrder, bool centerObject)
    : Component(associatedObject), centered(centerObject), renderLayer(renderLayer), renderOrder(renderOrder)
{
}

// Constructor with image file name
SpriteRenderer::SpriteRenderer(GameObject &associatedObject, const std::string fileName, RenderLayer renderLayer, int renderOrder, bool centerObject) : SpriteRenderer(associatedObject, renderLayer, renderOrder, centerObject)
{
  sprite = Resources::GetSprite(fileName);
}

void SpriteRenderer::Render() { Render(gameObject.GetPosition()); }

Vector2 SpriteRenderer::RenderPositionFor(Vector2 position, shared_ptr<Sprite> referenceSprite) const
{
  // Apply offset
  position += offset * gameObject.GetScale();

  // Offset coordinates if centered
  if (centered == false)
    return position;

  auto scale = gameObject.GetScale().GetAbsolute();
  auto sprite = referenceSprite == nullptr ? this->sprite : referenceSprite;

  // Get sprite's dimensions
  auto [width, height] = make_pair(sprite->GetWidth(scale.x), sprite->GetHeight(scale.y));

  return position - Vector2(width / 2, height / 2);
}

void SpriteRenderer::Render(Vector2 position)
{
  position = RenderPositionFor(position);

  auto scale = gameObject.GetScale().GetAbsolute();

  auto [width, height] = make_pair(sprite->GetWidth(scale.x), sprite->GetHeight(scale.y));

  // Get the real position box
  SDL_Rect destinationRect = (SDL_Rect)Camera::GetMain()->WorldToScreen(
      Rectangle{Rectangle::TopLeftInitialize, position, width, height});

  // Detect flips
  SDL_RendererFlip horizontalFlip = gameObject.localScale.x < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
  SDL_RendererFlip verticalFlip = gameObject.localScale.y < 0 ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;

  // Get source clip
  auto sourceRect = sprite->GetClip();

  // Put the texture in the renderer
  SDL_RenderCopyEx(
      Game::GetInstance().GetRenderer(),
      sprite->GetTexture().get(),
      &sourceRect,
      &destinationRect,
      Helper::RadiansToDegrees(gameObject.GetRotation()),
      nullptr,
      SDL_RendererFlip(horizontalFlip | verticalFlip));
}
