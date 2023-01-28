#ifndef __SPRITE__
#define __SPRITE__

#include <memory>
#include <string>
#include "SDL_image.h"
#include "Rectangle.h"
#include "Game.h"

// Holds parameters for configuring a sprite
struct SpriteConfig
{
  // The size of a pixel from this image relative to a game unit
  float pixelsPerUnit;

  // When both target dimensions are -1, uses the clip's dimensions. If only one is -1, the correspondent aspect ratio dimensions is used
  // Target width of image
  int targetWidth{-1};

  // Target height of image
  int targetHeight{-1};

  SpriteConfig() : SpriteConfig(Game::defaultVirtualPixelsPerUnit) {}
  SpriteConfig(float ppu) : pixelsPerUnit(ppu) {}
};

// Defines a data type that holds a sprite, already clipped
class Sprite
{
public:
  Sprite(const std::string initialTexture, SpriteConfig config = SpriteConfig());

  SDL_Rect GetClip() const;

  std::shared_ptr<SDL_Texture> GetTexture() const;

  // Sets which rectangle of the image is to be displayed
  void SetClip(SDL_Rect rect);

  // Set the dimensions of the image to be displayed (in game units, NOT pixels)
  void SetTargetDimension(int width, int height = -1);

  // Get width of the sprite's image, in real pixels
  int GetUnscaledWidth() const { return clipRect.w; }

  // Get height of the sprite's image, in real pixels
  int GetUnscaledHeight() const { return clipRect.h; }

  // Get width of sprite scaled by worldObject's scale, in units
  float GetWidth(float scale = 1) const;

  // Get height of sprite scaled by worldObject's scale, in units
  float GetHeight(float scale = 1) const;

  void SetConfig(SpriteConfig config);

private:
  // Loads the file image to the sprite
  void Load(const std::string fileName);

  SpriteConfig config;

  // The loaded texture
  std::shared_ptr<SDL_Texture> texture;

  // Dimensions of the texture, in real pixels
  int width, height;

  // The clipped rectangle of the image to be rendered
  SDL_Rect clipRect;
};

#endif