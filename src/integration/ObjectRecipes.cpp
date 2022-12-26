#include "ObjectRecipes.h"
#include "CameraFollower.h"
#include "Animator.h"
#include "Sound.h"
#include "Camera.h"
#include "Game.h"
#include "Rigidbody.h"
#include "Rectangle.h"
#include "Movement.h"
#include "KeyboardInput.h"
#include "ControllerInput.h"
#include "Animator.h"
#include "AnimationRecipes.h"
#include "CharacterController.h"
#include "Character.h"
#include "ParticleEmitter.h"
#include "PlayerManager.h"
#include <iostream>

using namespace std;

auto ObjectRecipes::Camera(float size) -> function<void(shared_ptr<GameObject>)>
{
  return [size](shared_ptr<GameObject> cameraObject)
  {
    cameraObject->AddComponent<::Camera>(size);
  };
}

auto ObjectRecipes::PlayerManager() -> function<void(shared_ptr<GameObject>)>
{
  return [](shared_ptr<GameObject> managerObject)
  {
    managerObject->AddComponent<::PlayerManager>();
    managerObject->DontDestroyOnLoad();
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
      sprite->SetTargetDimension(Game::screenWidth / Camera::GetMain()->GetRealPixelsPerUnit());
    }
    else
    {
      sprite->SetTargetDimension(-1, Game::screenHeight / Camera::GetMain()->GetRealPixelsPerUnit());
    }
  };
}

auto ObjectRecipes::Character(shared_ptr<Player> player) -> std::function<void(std::shared_ptr<GameObject>)>
{
  return [player](shared_ptr<GameObject> character)
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

    // Give it a dash particle emitter
    auto dashParticleObject = character->CreateChild(DASH_PARTICLES_OBJECT);
    auto particleEmitter = dashParticleObject->AddComponent<ParticleEmitter>(RenderLayer::VFX, 0.25, true, 5);
    particleEmitter->SetOffset({0, 0.25});
    particleEmitter->emission.color = {Color::Black(), Color::Gray()};
    particleEmitter->emission.frequency = {0.0001, 0.001};
    particleEmitter->emission.speed = {0.01, 0.1};
    particleEmitter->emission.lifetime = {0.2, 0.6};
    particleEmitter->emitOnStart = false;
    // Reduce frequency over emission
    particleEmitter->emissionEvolution = [](ParticleEmissionParameters &params, float deltaTime)
    { auto reduction = 0.5 * deltaTime;
      params.frequency = {params.frequency.first + reduction,
                          params.frequency.second + reduction}; };

    // Give it movement
    character->AddComponent<::Character>();
    character->AddComponent<Movement>(35, 5, collider->GetBox().height / 2);
    character->AddComponent<ControllerInput>(player);
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
