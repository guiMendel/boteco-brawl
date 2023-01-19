#ifndef __SPRITE_RENDERER__
#define __SPRITE_RENDERER__

#include <memory>
#include <SDL.h>
#include <SDL_image.h>
#include "Helper.h"
#include "GameObject.h"
#include "Component.h"
#include "Sprite.h"

class SpriteRenderer : public Component
{
public:
  // Raised whenever offset changes
  EventI<Vector2> OnSetOffset;

  // Constructor with image file name
  SpriteRenderer(GameObject &associatedObject, const std::string fileName, RenderLayer renderLayer = RenderLayer::Default, int renderOrder = 0, bool centerObject = true);

  // Default constructor
  SpriteRenderer(GameObject &associatedObject, RenderLayer renderLayer = RenderLayer::Default, int renderOrder = 0, bool centerObject = true);

  virtual ~SpriteRenderer() {}

  // Renders the sprite using the associated object's position
  void Render() override;

  // Given a render position, returns the position where the top-left pixel will be rendered
  Vector2 RenderPositionFor(Vector2 position, std::shared_ptr<Sprite> referenceSprite = nullptr) const;

  // Renders the sprite to the provided position, ignoring the associated object's position
  void Render(Vector2 position);

  void Update([[maybe_unused]] float deltaTime) override {}

  RenderLayer GetRenderLayer() override { return renderLayer; }

  int GetRenderOrder() override { return renderOrder; }

  Vector2 GetOffset() const;

  void SetOffset(Vector2 newOffset);

  // Which sprite is currently loaded
  std::shared_ptr<Sprite> sprite;

  // Whether to center the sprite on the render coordinates
  bool centered;

private:
  // The sprite's layer
  RenderLayer renderLayer{RenderLayer::Default};

  // Offset when rendering based on game object's position
  Vector2 offset;

  // The sprite's render order
  int renderOrder{0};
};

#endif