#include "UIText.h"
#include "Resources.h"
#include "Camera.h"

using namespace std;

UIText::UIText(Canvas &canvas, string name, std::shared_ptr<UIContainer> parent, string text)
    : UIContent(canvas, name, parent), text(text), mainTexture(nullptr, SDL_DestroyTexture) {}

void UIText::Start()
{
  // Load texture format through a sample texture
  auto sampleTexture = GetTextureWithColor(Color::White());

  SDL_QueryTexture(sampleTexture.get(), &textureFormat, nullptr, nullptr, nullptr);

  // Initialize texture
  RemakeTexture();

  // Remake whenever a text related style property changes
  auto remake = [this]()
  { RemakeTexture(); };

  style->fontPath.OnChangeValue.AddListener("remake-text", remake);
  style->fontSize.OnChangeValue.AddListener("remake-text", remake);
  style->textColor.OnChangeValue.AddListener("remake-text", remake);
  style->textBorderColor.OnChangeValue.AddListener("remake-text", remake);
  style->textBorderSize.OnChangeValue.AddListener("remake-text", remake);

  UIObject::Start();
}

void UIText::Render()
{
  // Get the real position
  // Offset coordinates to match anchor point
  Vector2 realPosition{canvas.CanvasToScreen(GetContentPosition())};

  SDL_Rect destinationRect{int(realPosition.x), int(realPosition.y), pixelWidth, pixelHeight};

  // Get clip rectangle
  SDL_Rect clipRect{0, 0, pixelWidth, pixelHeight};

  // Put the texture in the renderer
  SDL_RenderCopy(
      Game::GetInstance().GetRenderer(),
      mainTexture.get(),
      &clipRect,
      &destinationRect);

  // Debug render
  UIObject::Render();
}

void UIText::Update(float deltaTime)
{
  if (forceRemake)
  {
    RemakeTexture();
    forceRemake = false;
  }

  UIContent::Update(deltaTime);
}

void UIText::SetText(string text)
{
  this->text = text;
  RemakeTexture();
}

auto_unique_ptr<SDL_Texture> UIText::GetTextureWithColor(Color targetColor)
{
  // Get font properties
  auto fontPath = style->fontPath.Get();
  auto fontSize = style->fontSize.Get();

  // Get font
  auto font = Resources::GetFont(fontPath, fontSize);

  // Will hold the generated surface
  auto_unique_ptr<SDL_Surface> surface(
      TTF_RenderText_Solid(font.get(), text.c_str(), targetColor),
      SDL_FreeSurface);

  // Ensure it's loaded
  Assert(surface != nullptr, "Failed to generate surface from font");

  // Convert to texture
  return {
      SDL_CreateTextureFromSurface(Game::GetInstance().GetRenderer(), surface.get()),
      SDL_DestroyTexture};
}

void UIText::RemakeTexture()
{
  // Get properties
  auto color = style->textColor.Get();
  auto borderPixels = style->textBorderSize.Get();
  auto borderColor = style->textBorderColor.Get();

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
  for (int row = 0; row <= borderPixels * 2; row++)
    for (int column = 0; column <= borderPixels * 2; column++)
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

string UIText::GetText() { return text; }

int UIText::GetContentRealPixelsAlong(UIDimension::Axis axis, UIDimension::Calculation)
{
  return axis == UIDimension::Horizontal ? pixelWidth : pixelHeight;
}
