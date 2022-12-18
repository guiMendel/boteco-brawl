#include "ObjectRecipes.h"
#include "CameraFollower.h"
#include "Animator.h"
#include "Sound.h"
#include "Camera.h"
#include "Game.h"
#include "Rigidbody.h"
#include "Rectangle.h"
#include "Movement.h"
#include "PlayerInput.h"
#include "Animator.h"
#include "AnimationRecipes.h"
#include "CharacterController.h"
#include "Character.h"
#include <iostream>

using namespace std;

auto ObjectRecipes::Camera(float size) -> function<void(shared_ptr<GameObject>)>
{
  return [size](shared_ptr<GameObject> cameraObject)
  {
    cameraObject->AddComponent<::Camera>(size);
  };
}

auto ObjectRecipes::Background(string imagePath) -> function<void(shared_ptr<GameObject>)>
{
  return [imagePath](shared_ptr<GameObject> background)
  {
    // Get a background sprite
    auto sprite = background->AddComponent<SpriteRenderer>(imagePath, RenderLayer::Background)->sprite;

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

auto ObjectRecipes::Character() -> std::function<void(std::shared_ptr<GameObject>)>
{
  return [](shared_ptr<GameObject> character)
  {
    // Get sprite
    auto spriteRenderer = character->AddComponent<SpriteRenderer>(RenderLayer::Characters);

    // Add animator
    auto animator = character->AddComponent<Animator>();

    // Add animations
    animator->AddAnimation(AnimationRecipes::Idle);
    animator->AddAnimation(AnimationRecipes::Run);
    animator->AddAnimation(AnimationRecipes::Jump);
    animator->AddAnimation(AnimationRecipes::Rise);
    animator->AddAnimation(AnimationRecipes::Fall);
    animator->AddAnimation(AnimationRecipes::Land);
    animator->AddAnimation(AnimationRecipes::Brake);
    animator->AddAnimation(AnimationRecipes::Punch);
    animator->AddAnimation(AnimationRecipes::Dash);

    // Give it collision
    auto body = character->AddComponent<Rigidbody>(RigidbodyType::Dynamic, 0, 0);
    auto collider = character->AddComponent<Collider>(animator, false, ColliderDensity::Character);

    // Turn on continuous collision
    body->continuousCollisions = true;

    // Give it movement
    character->AddComponent<::Character>();
    character->AddComponent<Movement>(35, 5, collider->GetBox().height / 2);
    character->AddComponent<PlayerInput>();
    character->AddComponent<CharacterController>();
  };
}

auto ObjectRecipes::Platform(Vector2 size, bool isStatic) -> std::function<void(std::shared_ptr<GameObject>)>
{
  return [size, isStatic](shared_ptr<GameObject> platform)
  {
    platform->AddComponent<Rigidbody>(isStatic ? RigidbodyType::Static : RigidbodyType::Dynamic);
    platform->AddComponent<Collider>(Rectangle(0, 0, size.x, size.y), false, ColliderDensity::Ground);
  };
}
