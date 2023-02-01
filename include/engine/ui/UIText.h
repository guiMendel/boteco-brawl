#ifndef __UI_TEXT__
#define __UI_TEXT__

#include "UIContent.h"
#include "Color.h"
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <memory>

class UIText : public UIContent
{
public:
  UIText(Canvas &canvas, std::string name, std::string text = "");

  virtual ~UIText() {}

  void Render() override;

  // Sets new text content
  void SetText(std::string text);

  // Gets current text content
  std::string GetText();

private:
  // Recreates the main texture, applying a border
  void RemakeTexture();

  // Gets a simple texture for the current text, with the specified color
  auto_unique_ptr<SDL_Texture> GetTextureWithColor(Color targetColor);

  // Which text to show
  std::string text;

  // Texture of text
  auto_unique_ptr<SDL_Texture> mainTexture;

  // Quick access to texture width
  int pixelWidth{0};

  // Quick access to texture height
  int pixelHeight{0};

  // Format to use when creating textures
  Uint32 textureFormat;
};

#endif