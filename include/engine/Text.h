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
    solid,
    // With black background
    shaded,
    // With smoothed edges (more resource intensive)
    blended
  };

  Text(
      GameObject &associatedObject, std::string text, std::string fontPath,
      int size = 10, Style style = Style::solid, Color color = Color::White());

  virtual ~Text() {}

  void Render() override;
  RenderLayer GetRenderLayer() override { return RenderLayer::UI; }

  void SetFontFile(const std::string fontPath);

  int GetWidth() const { return width; }
  int GetHeight() const { return height; }

  void SetText(std::string text);
  void SetColor(Color color);
  void SetStyle(Style style);
  void SetFontSize(int fontSize);

  // Getters aren't necessary yet so weren't implemented

private:
  // Remakes the texture according to the new text settings
  void RemakeTexture();

  // Which text to show
  std::string text;

  // The size of the font
  int fontSize;

  // The text's style
  Style style;

  // Color of the font
  Color color;

  // The file path to the font
  std::string fontPath;

  // Font to use
  std::shared_ptr<TTF_Font> font;

  // Texture of text
  auto_unique_ptr<SDL_Texture> texture;

  // Quick access to texture width
  int width{0};

  // Quick access to texture height
  int height{0};
};

#endif