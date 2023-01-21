#ifndef __TEXT__
#define __TEXT__

#include "GameObject.h"
#include "Component.h"
#include "Color.h"
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <memory>

class Text : public Component
{
public:
  // Style of text
  enum class Style
  {
    // Default text
    Solid,
    // With black background
    Shaded,
    // With smoothed edges (more resource intensive)
    Blended
  };

  Text(
      GameObject &associatedObject, std::string text, std::string fontPath,
      int size = 10, Style style = Style::Solid, Color color = Color::White());

  virtual ~Text() {}

  void Render() override;
  RenderLayer GetRenderLayer() override { return RenderLayer::UI; }

  void SetFontFile(const std::string fontPath);

  int GetWidth() const { return pixelWidth; }
  int GetHeight() const { return pixelHeight; }

  void SetText(std::string text);
  void SetColor(Color color);
  void SetStyle(Style style);
  void SetFontSize(int fontSize);
  void SetBorderSize(int borderSize);
  void SetBorderColor(Color borderColor);

  // Getters aren't necessary yet so weren't implemented

private:
  // Recreates the main texture, applying a border
  void RemakeTexture();

  // Gets a simple texture for the current text, with the specified color
  auto_unique_ptr<SDL_Texture> GetTextureWithColor(Color targetColor);

  // Which text to show
  std::string text;

  // The size of the font
  int fontSize;

  // The text's style
  Style style;

  // Color of the font
  Color color;

  // Pixel size of border
  size_t borderPixels{0};

  // Color of border
  Color borderColor{Color::Black()};

  // The file path to the font
  std::string fontPath;

  // Font to use
  std::shared_ptr<TTF_Font> font;

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