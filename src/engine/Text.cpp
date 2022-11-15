#include "Text.h"
#include "Resources.h"
#include "Camera.h"

using namespace std;

Text::Text(
    GameObject &associatedObject, string text, string fontPath,
    int size, Style style, Color color)
    : Component(associatedObject), text(text), fontSize(size),
      style(style), color(color), fontPath(fontPath), font(Resources::GetFont(fontPath, size)), texture(nullptr, SDL_DestroyTexture)
{
  // Initialize texture
  RemakeTexture();
}

void Text::Render()
{
  // Offset coordinates to center texture
  Vector2 position = gameObject.GetPosition() - Vector2((float)width, (float)height) / 2;

  // Get the real position
  Vector2 offsetPosition = Camera::GetInstance().WorldToScreen(position);

  // Get clip rectangle
  SDL_Rect clipRect{0, 0, width, height};

  // Get destination rectangle
  SDL_Rect destinationRect{(int)offsetPosition.x, (int)offsetPosition.y, width, height};

  // Put the texture in the renderer
  SDL_RenderCopyEx(
      Game::GetInstance().GetRenderer(),
      texture.get(),
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
  this->fontSize = fontSize;
  RemakeTexture();
}

void Text::RemakeTexture()
{
  // Will hold the generated surface
  auto_unique_ptr<SDL_Surface> surface(nullptr, SDL_FreeSurface);

  // Use the appropriate method to load this
  if (style == Style::solid)
    surface.reset(TTF_RenderText_Solid(font.get(), text.c_str(), color));
  else if (style == Style::shaded)
    surface.reset(TTF_RenderText_Shaded(font.get(), text.c_str(), color, Color::Black()));
  else
    surface.reset(TTF_RenderText_Blended(font.get(), text.c_str(), color));

  // Ensure it's loaded
  Assert(surface != nullptr, "Failed to generate surface from font");

  // Get the dimensions
  width = surface->w;
  height = surface->h;

  // Convert to texture
  texture.reset(
      SDL_CreateTextureFromSurface(Game::GetInstance().GetRenderer(), surface.get()));
}
