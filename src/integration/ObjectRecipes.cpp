#include "ObjectRecipes.h"
#include "LandingAttackEffector.h"
#include "Arena.h"
#include "FallOffDeath.h"
#include "PlatformEffector.h"
#include "TestCharacter.h"
#include "PlatformDrop.h"
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
#include "GeneralAnimations.h"
#include "CharacterController.h"
#include "CharacterStateManager.h"
#include "ParticleEmitter.h"
#include "PlayerManager.h"
#include "Projectile.h"
#include "CharacterRepelCollision.h"
#include "BoxCollider.h"
#include "CircleCollider.h"
#include "Circle.h"
#include "Heat.h"
#include "GunParry.h"
#include <iostream>

#define JUMP_RANGE 0.2f

using namespace std;

auto ObjectRecipes::Camera(float size) -> function<void(shared_ptr<GameObject>)>
{
  return [size](shared_ptr<GameObject> cameraObject)
  {
    cameraObject->AddComponent<::Camera>(size);
  };
}

auto ObjectRecipes::Arena(string imagePath) -> function<void(shared_ptr<GameObject>)>
{
  return [imagePath](shared_ptr<GameObject> arena)
  {
    // Get a background sprite
    auto spriteRenderer = arena->AddComponent<SpriteRenderer>(imagePath, RenderLayer::Background);
    auto sprite = spriteRenderer->sprite;

    // Make it cover the screen
    if (sprite->GetWidth() < sprite->GetHeight())
      sprite->SetTargetDimension(Game::screenWidth / Camera::GetMain()->GetRealPixelsPerUnit());
    else
      sprite->SetTargetDimension(-1, Game::screenHeight / Camera::GetMain()->GetRealPixelsPerUnit());

    // Add arena
    arena->AddComponent<::Arena>(spriteRenderer);
  };
}

auto ObjectRecipes::CharacterStateManager(shared_ptr<Player> player) -> function<void(shared_ptr<GameObject>)>
{
  auto weakPlayer{weak_ptr(player)};
  return [weakPlayer](shared_ptr<GameObject> character)
  {
    character->SetPhysicsLayer(PhysicsLayer::Character);

    IF_NOT_LOCK(weakPlayer, player) { return; }

    // === RENDERING

    // Get sprite
    auto spriteRenderer = character->AddComponent<SpriteRenderer>(RenderLayer::Characters);

    // Add animator
    auto animator = character->AddComponent<Animator>();

    // Add animations
    animator->RegisterAnimation<GeneralAnimations::Idle>();
    animator->RegisterAnimation<GeneralAnimations::Run>();
    animator->RegisterAnimation<GeneralAnimations::Jump>();
    animator->RegisterAnimation<GeneralAnimations::Rise>();
    animator->RegisterAnimation<GeneralAnimations::Fall>();
    animator->RegisterAnimation<GeneralAnimations::Land>();
    animator->RegisterAnimation<GeneralAnimations::Brake>();
    animator->RegisterAnimation<GeneralAnimations::Ouch1>();
    animator->RegisterAnimation<GeneralAnimations::Ouch2>();
    animator->RegisterAnimation<GeneralAnimations::Dash>();
    animator->RegisterAnimation<GeneralAnimations::Neutral1>();
    animator->RegisterAnimation<GeneralAnimations::Neutral2>();
    animator->RegisterAnimation<GeneralAnimations::Horizontal>();
    animator->RegisterAnimation<GeneralAnimations::Up>();
    animator->RegisterAnimation<GeneralAnimations::AirHorizontal>();
    animator->RegisterAnimation<GeneralAnimations::AirUp>();
    animator->RegisterAnimation<GeneralAnimations::AirDown>();
    animator->RegisterAnimation<GeneralAnimations::SpecialNeutral>();
    animator->RegisterAnimation<GeneralAnimations::Riposte>();
    animator->RegisterAnimation<GeneralAnimations::LandingAttack>();

    // === COLLISION

    // Give it collision
    auto body = character->AddComponent<Rigidbody>(RigidbodyType::Dynamic, 0, 0);
    auto collider = character->AddComponent<BoxCollider>(animator, false, ColliderDensity::Character);

    // Turn on continuous collision
    body->continuousCollisions = true;

    // === PARTICLE EFFECTS

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

    // === DYING FROM FALLING OFF

    character->AddComponent<FallOffDeath>();

    // === MOVEMENT CONTROL

    // Give it movement
    character->AddComponent<::CharacterStateManager>();
    character->AddComponent<Movement>(35, 5, collider->GetBox().height / 2);

    // Give it input
    if (player->IsMain())
      character->AddComponent<KeyboardInput>();
    else
      character->AddComponent<ControllerInput>(player);

    // Give it control
    character->AddComponent<CharacterController>();

    // === CHARACTER REPELLING

    // Give it a repel box trigger collider, to allow for making 2 characters slide away from each other when overlapping
    auto repelBox = character->CreateChild(CHARACTER_SLIDE_BOX_OBJECT)->AddComponent<BoxCollider>(collider, true);

    // Make it a different layer so that they collide
    repelBox->gameObject.AddComponent<CharacterRepelCollision>(body);
    repelBox->gameObject.SetPhysicsLayer(PhysicsLayer::CharacterRepelBox);

    // === PLATFORM DETECTOR FOR DROPPING

    // Give it a repel box trigger collider, to allow for making 2 characters slide away from each other when overlapping
    auto platformDropDetector = character->CreateChild(CHARACTER_PLATFORM_DROP_OBJECT);
    platformDropDetector->AddComponent<PlatformDrop>(body);

    // Rectangle for it's collider: a bit bigger than the regular collider
    platformDropDetector->AddComponent<BoxCollider>(collider, true, ColliderDensity::Ground, Vector2::One() * 1.2);

    // Make it a different layer so that they collide only with platforms
    platformDropDetector->SetPhysicsLayer(PhysicsLayer::CharacterPlatformDrop);

    // === ATTACKING

    // Give it a character to differentiate which animations to play for each attack
    character->AddComponent<TestCharacter>();

    // Make it vulnerable
    character->AddComponent<Heat>(1);

    // Give it parry capacity
    character->AddComponent<GunParry>();

    // === LANDING ATTACKS

    auto weakAnimator{weak_ptr(animator)};

    // Decides whether the landing effector should trigger on land
    auto effectorCondition = [weakAnimator]()
    {
      LOCK(weakAnimator, animator);

      // Yes if the current animation is the shovel drop
      return animator->GetCurrentAnimation()->Name() == AIR_DOWN_SHOVEL_LOOP;
    };

    // Give it the landing attack effector
    character->AddComponent<LandingAttackEffector>(effectorCondition);
  };
}

auto ObjectRecipes::Platform(Vector2 size, bool withEffector) -> function<void(shared_ptr<GameObject>)>
{
  return [size, withEffector](shared_ptr<GameObject> platform)
  {
    platform->AddComponent<BoxCollider>(Rectangle({0, 0}, size.x, size.y), false, ColliderDensity::Ground);

    if (withEffector)
    {
      platform->AddComponent<PlatformEffector>();
      platform->SetPhysicsLayer(PhysicsLayer::Platform);
      platform->AddComponent<Rigidbody>(RigidbodyType::Kinematic);
    }
    else
    {
      platform->SetPhysicsLayer(PhysicsLayer::Scenario);
      platform->AddComponent<Rigidbody>(RigidbodyType::Static);
    }
  };
}

auto ObjectRecipes::Projectile(Vector2 initialVelocity, shared_ptr<GameObject> parent) -> function<void(shared_ptr<GameObject>)>
{
  auto weakParent{weak_ptr(parent)};
  return [initialVelocity, weakParent](shared_ptr<GameObject> projectile)
  {
    projectile->SetPhysicsLayer(PhysicsLayer::Hazard);
    IF_NOT_LOCK(weakParent, parent) { return; }

    auto body = projectile->AddComponent<Rigidbody>(RigidbodyType::Dynamic);
    // auto collider = projectile->AddComponent<BoxCollider>(Rectangle({0, 0}, 0.2, 0.2), true, ColliderDensity::Wood);
    auto collider = projectile->AddComponent<CircleCollider>(Circle({0, 0}, 0.2), true, ColliderDensity::Wood);
    projectile->AddComponent<::Projectile>(parent);

    body->velocity = initialVelocity;
  };
}
