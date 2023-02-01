#ifndef __UI_IMAGE__
#define __UI_IMAGE__

#include "UIContent.h"

// A UI Object which can contain children UI Objects
class UIImage : public UIContent
{
public:
  UIImage(Canvas &canvas, std::string name, std::string imagePath);

  virtual ~UIImage() {}

  void Render() override;

  // Sets new image path
  void SetImagePath(std::string imagePath);

  // Gets current image path
  std::string GetImagePath();

private:
  // Allows a procedure to operate with a texture from image with color alterations already applied to it
  void UseTexture(std::function<void(SDL_Texture *)> procedure);

  // Loads texture dimensions from the current imagePath
  void ReloadTextureDimensions();

  // Which image to show
  std::string imagePath;

  // Original real pixel width of the loaded texture
  int textureWidth;

  // Original real pixel height of the loaded texture
  int textureHeight;
};

#endif