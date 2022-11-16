#ifndef __SPRITE__
#define __SPRITE__

#include <memory>
#include <SDL.h>
#include <SDL_image.h>
#include "Helper.h"
#include "GameObject.h"
#include "Component.h"

class Sprite : public Component
{
public:
  // Since we are using unique ptrs, no need to define destructor

  // Default constructor
  Sprite(GameObject &associatedObject, RenderLayer renderLayer = RenderLayer::Default, float pixelsPerUnit = 32, int renderOrder = 0, bool centerObject = true);

  // Constructor with image file name
  Sprite(GameObject &associatedObject, const std::string fileName, RenderLayer renderLayer = RenderLayer::Default, float pixelsPerUnit = 32, int renderOrder = 0, bool centerObject = true);

  virtual ~Sprite() {}

  // Loads the file image to the sprite
  void Load(const std::string fileName);

  // Sets which rectangle of the image is to be displayed
  void SetClip(int x, int y, int width, int height);

  int GetUnscaledWidth() const { return clipRect.w; }

  int GetUnscaledHeight() const { return clipRect.h; }

  float GetWidth() const;

  float GetHeight() const;

  // Set the dimensions of the image to be displayed (in game units, NOT pixels)
  void SetTargetDimension(int width, int height = -1);

  bool IsLoaded() const { return texture != nullptr; }

  // Renders the sprite using the associated object's position
  void Render() override { Render(gameObject.GetPosition() + offset); }

  // Renders the sprite to the provided position, ignoring the associated object's position
  void Render(Vector2 position);

  void Update([[maybe_unused]] float deltaTime) override {}

  RenderLayer GetRenderLayer() override { return renderLayer; }

  int GetRenderOrder() override { return renderOrder; }

  // Offset when rendering based on game object's position
  Vector2 offset{0, 0};

  // Whether to center the sprite on the render coordinates
  bool centered;

  // The size of a pixel from this image relative to a game unit
  float pixelsPerUnit;

private:
  // The loaded texture
  std::shared_ptr<SDL_Texture> texture;

  // The clipped rectangle of the image to be rendered
  SDL_Rect clipRect;

  // When both target dimensions are -1, uses the clip's dimensions. If only one is -1, the correspondent aspect ratio dimensions is used
  // Target width of image
  int targetWidth{-1};

  // Target height of image
  int targetHeight{-1};

  // The sprite's layer
  RenderLayer renderLayer{RenderLayer::Default};

  // The sprite's render order
  int renderOrder{0};
};

#endif