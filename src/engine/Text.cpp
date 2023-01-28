#include "Text.h"
#include "Resources.h"
#include "Camera.h"

using namespace std;

Text::Text(
    GameObject &associatedObject, string text, string fontPath,
    int size, Style style, Color color)
    : Component(associatedObject), text(text), fontSize(size),
      style(style), color(color), fontPath(fontPath), font(Resources::GetFont(fontPath, size)), mainTexture(nullptr, SDL_DestroyTexture)
{
  // Load texture format through a sample texture
  auto sampleTexture = GetTextureWithColor(Color::White());

  SDL_QueryTexture(sampleTexture.get(), &textureFormat, nullptr, nullptr, nullptr);

  // Initialize texture
  RemakeTexture();
}

void Text::Render()
{
  // Get the real position
  // Offset coordinates to match anchor point
  Vector2 realPosition{
      Camera::GetMain()->WorldToScreen(gameObject.GetPosition()) -
      Vector2((float)pixelWidth, (float)pixelHeight) * anchorPoint +
      offset};

  SDL_Rect destinationRect{int(realPosition.x), int(realPosition.y), pixelWidth, pixelHeight};

  // Get clip rectangle
  SDL_Rect clipRect{0, 0, pixelWidth, pixelHeight};

  // Put the texture in the renderer
  SDL_RenderCopyEx(
      Game::GetInstance().GetRenderer(),
      mainTexture.get(),
      &clipRect,
      &destinationRect,
      Helper::RadiansToDegrees(gameObject.GetRotation()),
      nullptr,
      SDL_FLIP_NONE);
}

void Text::SetText(string text)
{
  this->text = text;
  RemakeTexture();
}

void Text::SetColor(Color color)
{
  this->color = color;
  RemakeTexture();
}

void Text::SetStyle(Style style)
{
  this->style = style;
  RemakeTexture();
}

void Text::SetFontSize(int fontSize)
{
  if (fontSize == this->fontSize)
    return;

  this->fontSize = fontSize;
  this->font = Resources::GetFont(fontPath, fontSize);
  RemakeTexture();
}

auto_unique_ptr<SDL_Texture> Text::GetTextureWithColor(Color targetColor)
{
  // Will hold the generated surface
  auto_unique_ptr<SDL_Surface> surface(nullptr, SDL_FreeSurface);

  // Use the appropriate method to load this
  if (style == Style::Solid)
    surface.reset(TTF_RenderText_Solid(font.get(), text.c_str(), targetColor));
  else if (style == Style::Shaded)
    surface.reset(TTF_RenderText_Shaded(font.get(), text.c_str(), targetColor, Color::Black()));
  else
    surface.reset(TTF_RenderText_Blended(font.get(), text.c_str(), targetColor));

  // Ensure it's loaded
  Assert(surface != nullptr, "Failed to generate surface from font");

  // Convert to texture
  return {
      SDL_CreateTextureFromSurface(Game::GetInstance().GetRenderer(), surface.get()),
      SDL_DestroyTexture};
}

void Text::RemakeTexture()
{
  // Pick texture color according to necessity of border
  Color firstColor = borderPixels == 0 ? color : borderColor;

  auto baseTexture = GetTextureWithColor(firstColor);

  // Store dimensions
  SDL_QueryTexture(baseTexture.get(), nullptr, nullptr, &pixelWidth, &pixelHeight);

  // If there's no border, we simply use this texture
  if (borderPixels == 0)
  {
    mainTexture.swap(baseTexture);
    return;
  }

  // Get renderer
  auto renderer = Game::GetInstance().GetRenderer();

  // Get clip for this texture
  SDL_Rect baseTextureClip{0, 0, pixelWidth, pixelHeight};

  // Add border size to dimensions
  pixelWidth += borderPixels * 2;
  pixelHeight += borderPixels * 2;

  // Create a texture where we can paint the borders
  mainTexture.reset(
      SDL_CreateTexture(renderer, textureFormat, SDL_TEXTUREACCESS_TARGET, pixelWidth, pixelHeight));

  // Set it as render target
  SDL_SetRenderTarget(renderer, mainTexture.get());

  // Set it transparent
  SDL_SetTextureBlendMode(mainTexture.get(), SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderFillRect(renderer, NULL);

  // Revert blend mode
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  // Start blipping border texture onto it
  for (size_t row = 0; row <= borderPixels * 2; row++)
    for (size_t column = 0; column <= borderPixels * 2; column++)
    {
      // Skip center
      if (row == borderPixels && column == borderPixels)
        continue;

      auto destination = baseTextureClip;
      destination.x += column;
      destination.y += row;

      SDL_RenderCopy(renderer, baseTexture.get(), &baseTextureClip, &destination);
    }

  // Get actual text texture
  auto textTexture = GetTextureWithColor(color);

  // Blip it to the center
  auto destination = baseTextureClip;
  destination.x += borderPixels;
  destination.y += borderPixels;

  SDL_RenderCopy(renderer, textTexture.get(), &baseTextureClip, &destination);

  // Set render target back to window
  SDL_SetRenderTarget(renderer, nullptr);
}

void Text::SetFontFile(const string fontPath)
{
  if (fontPath == this->fontPath)
    return;

  this->fontPath = fontPath;
  this->font = Resources::GetFont(fontPath, fontSize);

  RemakeTexture();
}

void Text::SetBorderSize(int borderSize)
{
  this->borderPixels = borderSize;
  RemakeTexture();
}

void Text::SetBorderColor(Color borderColor)
{
  this->borderColor = borderColor;
  RemakeTexture();
}

string Text::GetText() { return text; }
Color Text::GetColor() { return color; }
Text::Style Text::GetStyle() { return style; }
int Text::GetFontSize() { return fontSize; }
int Text::GetBorderSize() { return borderPixels; }
Color Text::GetBorderColor() { return borderColor; }

void Text::SetAnchorPoint(Vector2 point) { anchorPoint = point; }

void Text::SetOffset(Vector2 newOffset) { offset = newOffset; }
