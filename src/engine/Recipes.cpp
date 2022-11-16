#include "Recipes.h"
#include "CameraFollower.h"
#include "SpriteAnimator.h"
#include "Sound.h"
#include "Camera.h"
#include "Game.h"
#include <iostream>

using namespace std;

auto Recipes::Background(string imagePath) -> function<void(shared_ptr<GameObject>)>
{
  return [imagePath](shared_ptr<GameObject> background)
  {
    // Get a background sprite
    auto sprite = background->AddComponent<Sprite>(imagePath, RenderLayer::Background, 0);

    // Make it follow the camera
    background->AddComponent<CameraFollower>();

    // Make it cover the screen
    if (sprite->GetWidth() < sprite->GetHeight())
    {
      sprite->SetTargetDimension(Game::screenWidth);
    }
    else
    {
      sprite->SetTargetDimension(-1, Game::screenHeight);
    }
  };
}
