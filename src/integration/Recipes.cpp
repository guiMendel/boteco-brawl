#include "Recipes.h"
#include "CameraFollower.h"
#include "SpriteAnimator.h"
#include "Sound.h"
#include "Camera.h"
#include "Game.h"
#include "Rigidbody.h"
#include "Rectangle.h"
#include <iostream>

using namespace std;

auto Recipes::Background(string imagePath) -> function<void(shared_ptr<GameObject>)>
{
  return [imagePath](shared_ptr<GameObject> background)
  {
    // Get a background sprite
    auto sprite = background->AddComponent<Sprite>(imagePath, RenderLayer::Background);

    // Make it follow the camera
    background->AddComponent<CameraFollower>();

    // Make it cover the screen
    if (sprite->GetWidth() < sprite->GetHeight())
    {
      sprite->SetTargetDimension(Game::screenWidth / Camera::GetInstance().GetPixelsPerUnit());
    }
    else
    {
      sprite->SetTargetDimension(-1, Game::screenHeight / Camera::GetInstance().GetPixelsPerUnit());
    }
  };
}

auto Recipes::Character() -> std::function<void(std::shared_ptr<GameObject>)>
{
  return [](shared_ptr<GameObject> character)
  {
    // Get sprite
    auto sprite = character->AddComponent<Sprite>("./assets/image/character.png", RenderLayer::Characters);

    character->AddComponent<Rigidbody>(RigidbodyType::Dynamic);
    character->AddComponent<Collider>(sprite, false, ColliderDensity::Character);
  };
}

auto Recipes::Platform(Vector2 size) -> std::function<void(std::shared_ptr<GameObject>)>
{
  return [size](shared_ptr<GameObject> platform)
  {
    platform->AddComponent<Rigidbody>(RigidbodyType::Static);
    platform->AddComponent<Collider>(Rectangle(0, 0, size.x, size.y), false, ColliderDensity::Ground);
  };
}
