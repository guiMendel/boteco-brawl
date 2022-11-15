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
  Sprite(GameObject &associatedObject, RenderLayer renderLayer = RenderLayer::Default, int renderOrder = 0, bool centerObject = true)
      : Component(associatedObject), centered(centerObject), renderLayer(renderLayer), renderOrder(renderOrder) {}

  // Constructor with image file name
  Sprite(GameObject &associatedObject, const std::string fileName, RenderLayer renderLayer = RenderLayer::Default, int renderOrder = 0, bool centerObject = true) : Sprite(associatedObject, renderLayer, renderOrder, centerObject)
  {
    Load(fileName);
  }
  
  virtual ~Sprite() {}

  // Loads the file image to the sprite
  void Load(const std::string fileName);

  // Sets which rectangle of the image to be displayed
  void SetClip(int x, int y, int width, int height);

  int GetUnscaledWidth() const { return width; }

  int GetUnscaledHeight() const { return height; }

  int GetWidth() const { return clipRect.w * gameObject.localScale.x; }

  int GetHeight() const { return clipRect.h * gameObject.localScale.y; }

  bool IsLoaded() const { return texture != nullptr; }

  // Renders the sprite using the associated object's position
  void Render() override { Render(gameObject.GetPosition() + offset); }

  // Renders the sprite to the provided position, ignoring the associated object's position
  void Render(Vector2 position);

  void Update([[maybe_unused]] float deltaTime) override {}

  // Offset when rendering based on game object's position
  Vector2 offset{0, 0};

  // Whether to center the sprite on the render coordinates
  bool centered;

  RenderLayer GetRenderLayer() override { return renderLayer; }

  int GetRenderOrder() override { return renderOrder; }

private:
  // The loaded texture
  std::shared_ptr<SDL_Texture> texture;

  // Real dimensions of the loaded image
  int width{0}, height{0};

  // Scaled dimensions
  int scaledWidth{0}, scaledHeight{0};

  // The clipped rectangle of the image to be rendered
  SDL_Rect clipRect;

  // The sprite's layer
  RenderLayer renderLayer{RenderLayer::Default};

  // The sprite's render order
  int renderOrder{0};
};

#endif