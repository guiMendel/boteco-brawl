#ifndef __SPRITE_RENDERER__
#define __SPRITE_RENDERER__

#include <memory>
#include <SDL.h>
#include <SDL_image.h>
#include "Helper.h"
#include "WorldObject.h"
#include "WorldComponent.h"
#include "Sprite.h"
#include "Color.h"

class SpriteRenderer : public WorldComponent
{
public:
  // Raised whenever offset changes
  EventI<Vector2> OnSetOffset;

  // Constructor with image file name
  SpriteRenderer(GameObject &associatedObject, const std::string fileName, RenderLayer renderLayer = RenderLayer::Default, int renderOrder = 0);

  // Default constructor
  SpriteRenderer(GameObject &associatedObject, RenderLayer renderLayer = RenderLayer::Default, int renderOrder = 0);

  virtual ~SpriteRenderer() {}

  // Renders the sprite using the associated object's position
  void Render() override;

  // Given a render position, returns the position where the top-left pixel will be rendered
  // Takes parallax into account if set
  Vector2 RenderPositionFor(Vector2 position, std::shared_ptr<Sprite> referenceSprite = nullptr) const;

  // Renders the sprite to the provided position, ignoring the associated object's position
  void Render(Vector2 position);

  void Update(float) override {}
  RenderLayer GetRenderLayer() override { return renderLayer; }

  int GetRenderOrder() override { return renderOrder; }
  void SetRenderOrder(int newOrder);

  Vector2 GetOffset() const;

  void SetOffset(Vector2 newOffset);

  // Sets colors for modulation and addition
  void SetColor(Color modulateColor, Color addColor = {-1, -1, -1});

  // Gets current colors used for modulation and addition
  std::pair<Color, Color> GetColors() const;

  // Get currently loaded sprite
  std::shared_ptr<Sprite> GetSprite() const;

  // Sets a new sprite
  void SetSprite(std::shared_ptr<Sprite> newSprite);

  // Sets which point of the sprite will be at the sprite renderer's position (default is center)
  void SetAnchorPoint(Vector2 point);

  // Sets a new width to render with, in real pixels
  void OverrideWidthPixels(int newWidth);

  // Get world units position (relative to this object) of the given virtual pixel of the provided sprite
  Vector2 GetVirtualPixelOffset(Vector2 virtualPixel, std::shared_ptr<Sprite> referenceSprite = nullptr) const;

  // Get dimensions of current sprite (in units) with parallax applied
  std::pair<float, float> GetSpriteDimensionsParallax(std::shared_ptr<Sprite> referenceSprite = nullptr) const;

  // Set a parallax modifier for the renderer
  void SetParallax(float parallax, float referenceCameraSize);

private:
  // Allows a procedure to operate with a texture from sprite with color alterations already applied to it
  void UseTexture(std::function<void(SDL_Texture *)> procedure);

  // Apply parallax to a given position
  Vector2 ApplyParallax(Vector2 position) const;

  // Last texture generated by generate texture
  SDL_Texture *lastTexture{nullptr};

  // Which sprite is currently loaded
  std::shared_ptr<Sprite> sprite;

  // Color to modulate sprite with
  Color modulateColor{Color::White()};

  // Color to add to sprite
  Color addColor{Color::Black()};

  // The sprite's layer
  RenderLayer renderLayer{RenderLayer::Default};

  // Offset when rendering based on world object's position
  Vector2 offset;

  // The sprite's render order
  int renderOrder{0};

  // The width with which to render the sprite, in real pixels (used mostly for UI)
  int overrideWidthPixels{-1};

  // Which point of the sprite will be at the center (default is center)
  Vector2 anchorPoint{0.5, 0.5};

  // Amount of parallax to apply (to position & size)
  // Must be in range [0, 1], where 1 means it's not affected by camera movement at all
  float parallax{0};

  // Reference camera size for applying parallax to dimensions
  float parallaxReferenceCameraSize;
};

#endif