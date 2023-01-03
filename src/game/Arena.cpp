#include "Arena.h"

using namespace std;

Arena::Arena(GameObject &associatedObject, std::shared_ptr<SpriteRenderer> backgroundRenderer)
    : Component(associatedObject),
      width(backgroundRenderer->sprite->GetWidth()),
      height(backgroundRenderer->sprite->GetHeight()),
      weakBackgroundRenderer(backgroundRenderer) {}

shared_ptr<SpriteRenderer> Arena::GetBackgroundRenderer() const
{
  LOCK(weakBackgroundRenderer, renderer);
  return renderer;
}
