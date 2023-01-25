#include "Arena.h"
#include "Debug.h"

using namespace std;

Arena::Arena(GameObject &associatedObject, float width, float height)
    : Component(associatedObject), width(width), height(height) {}

Arena::Arena(GameObject &associatedObject, std::shared_ptr<SpriteRenderer> backgroundRenderer)
    : Arena(associatedObject, backgroundRenderer->GetSprite()->GetWidth(), backgroundRenderer->GetSprite()->GetHeight())
{
  weakBackgroundRenderer = backgroundRenderer;
}

void Arena::Render()
{
  auto box = Rectangle({0,0}, width, height);
  
  Debug::DrawBox(box, Color::Red());
  Debug::DrawBox(box * 0.95, Color::Red());
}
