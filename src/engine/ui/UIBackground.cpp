#include "UIBackground.h"

using namespace std;

UIBackground::UIBackground(GameObject &associatedObject, Color color)
    : UIComponent(associatedObject), color(color) {}

void UIBackground::Render()
{
  auto camera = Lock(uiObject.canvas.weakCamera);

  // cout << "back pos: " << uiObject.GetPosition() << endl;

  // Get rect representing this particle
  auto pixelPosition = uiObject.canvas.CanvasToScreen(uiObject.GetPosition());

  SDL_Rect objectBox = SDL_Rect{
      int(pixelPosition.x),
      int(pixelPosition.y),
      uiObject.width.AsRealPixels(),
      uiObject.height.AsRealPixels()};

  // Get renderer
  auto renderer = Game::GetInstance().GetRenderer();

  // Use color
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, color.alpha);

  // Fill a rect at this particle's position
  SDL_RenderFillRect(renderer, &objectBox);
}
