#include "Recipes.h"
#include "CameraFollower.h"
#include "SpriteAnimator.h"
#include "Sound.h"
#include <iostream>

using namespace std;

auto Recipes::Background(string imagePath) -> function<void(shared_ptr<GameObject>)>
{
  return [imagePath](shared_ptr<GameObject> background)
  {
    // Get a background sprite
    background->AddComponent<Sprite>(imagePath, RenderLayer::Background, 0, false);

    // Make it follow the camera
    background->AddComponent<CameraFollower>(true);
  };
}
