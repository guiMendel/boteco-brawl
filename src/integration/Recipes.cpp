#include "Recipes.h"
#include "CameraFollower.h"
#include "SpriteAnimator.h"
#include "Sound.h"
#include "Camera.h"
#include "Game.h"
#include "Rigidbody.h"
#include "Rectangle.h"
#include "Movement.h"
#include "PlayerController.h"
#include <iostream>

using namespace std;

auto Recipes::Camera(float size) -> function<void(shared_ptr<GameObject>)>
{
  return [size](shared_ptr<GameObject> cameraObject)
  {
    cameraObject->AddComponent<::Camera>(size);
  };
}

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
      sprite->SetTargetDimension(Game::screenWidth / Camera::GetMain()->GetPixelsPerUnit());
    }
    else
    {
      sprite->SetTargetDimension(-1, Game::screenHeight / Camera::GetMain()->GetPixelsPerUnit());
    }
  };
}

auto Recipes::Character() -> std::function<void(std::shared_ptr<GameObject>)>
{
  return [](shared_ptr<GameObject> character)
  {
    // Get sprite
    auto sprite = character->AddComponent<Sprite>("./assets/image/character.png", RenderLayer::Characters);

    auto body = character->AddComponent<Rigidbody>(RigidbodyType::Dynamic, 0, 0);
    auto collider = character->AddComponent<Collider>(sprite, false, ColliderDensity::Character);
    character->AddComponent<Movement>(35, 5, collider->GetBox().height / 2);
    character->AddComponent<PlayerController>();

    // Turn on continuous collision
    body->continuousCollisions = true;
  };
}

auto Recipes::Platform(Vector2 size, bool isStatic) -> std::function<void(std::shared_ptr<GameObject>)>
{
  return [size, isStatic](shared_ptr<GameObject> platform)
  {
    platform->AddComponent<Rigidbody>(isStatic ? RigidbodyType::Static : RigidbodyType::Dynamic);
    platform->AddComponent<Collider>(Rectangle(0, 0, size.x, size.y), false, ColliderDensity::Ground);
  };
}
