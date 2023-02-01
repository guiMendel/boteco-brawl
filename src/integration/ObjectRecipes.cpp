#include "Canvas.h"
#include "ObjectRecipes.h"
#include "Resources.h"
#include "CharacterVFX.h"
#include "ObjectRecipes.h"
#include "LandingAttackEffector.h"
#include "Arena.h"
#include "FallDeath.h"
#include "PlatformEffector.h"
#include "TestCharacter.h"
#include "UIText.h"
#include "UIImage.h"
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
#include "CharacterRepelCollision.h"
#include "BoxCollider.h"
#include "CircleCollider.h"
#include "Circle.h"
#include "Heat.h"
#include "GunParry.h"
#include "CameraBehavior.h"
#include "CharacterUIManager.h"
#include <iostream>

#define JUMP_RANGE 0.2f

using namespace std;

auto ObjectRecipes::Camera(shared_ptr<WorldObject> charactersParent)
    -> function<void(shared_ptr<WorldObject>)>
{
  auto weakParent{weak_ptr(charactersParent)};
  return [weakParent](shared_ptr<WorldObject> cameraObject)
  {
    LOCK(weakParent, charactersParent);

    auto camera = cameraObject->AddComponent<::Camera>(5);

    // Register it prematurely
    camera->RegisterToScene();

    cameraObject->AddComponent<CameraBehavior>(charactersParent);
  };
}

auto ObjectRecipes::Arena(string imagePath) -> function<void(shared_ptr<WorldObject>)>
{
  return [imagePath](shared_ptr<WorldObject> arena)
  {
    // Get a background sprite
    // auto spriteRenderer = arena->AddComponent<SpriteRenderer>(imagePath, RenderLayer::Background);
    // auto sprite = spriteRenderer->sprite;

    // // Make it cover the screen
    // if (sprite->GetWidth() < sprite->GetHeight())
    //   sprite->SetTargetDimension(Game::screenWidth / Camera::GetMain()->GetRealPixelsPerUnit());
    // else
    //   sprite->SetTargetDimension(-1, Game::screenHeight / Camera::GetMain()->GetRealPixelsPerUnit());

    // Add arena
    arena->AddComponent<::Arena>(50, 13);
    // arena->AddComponent<::Arena>(24, 12);
  };
}

auto ObjectRecipes::Character(shared_ptr<Player> player) -> function<void(shared_ptr<WorldObject>)>
{
  auto weakPlayer{weak_ptr(player)};
  return [weakPlayer](shared_ptr<WorldObject> character)
  {
    character->SetPhysicsLayer(PhysicsLayer::Character);

    LOCK(weakPlayer, player);

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
    animator->RegisterAnimation<GeneralAnimations::SpecialHorizontal>();
    animator->RegisterAnimation<GeneralAnimations::Riposte>();
    animator->RegisterAnimation<GeneralAnimations::LandingAttack>();
    animator->RegisterAnimation<GeneralAnimations::Crash>();
    animator->RegisterAnimation<GeneralAnimations::Spin>();

    // === COLLISION

    // Give it collision
    auto body = character->AddComponent<Rigidbody>(RigidbodyType::Dynamic, 0, 0);
    auto collider = character->AddComponent<BoxCollider>(animator, false, ColliderDensity::Character);

    // Turn on continuous collision
    body->continuousCollisions = true;

    // === PARTICLE EFFECTS

    // Give it VFX emitters
    character->AddComponent<CharacterVFX>();

    // === DYING FROM FALLING OFF

    auto fallDeath = character->AddComponent<FallDeath>();

    // === MOVEMENT CONTROL

    // Give it movement
    character->AddComponent<::CharacterStateManager>();
    character->AddComponent<Movement>(35, 5, collider->GetBox().height / 2);

    // Give it control
    character->AddComponent<CharacterController>(player);

    // Give it input
    if (player->IsMain())
      character->AddComponent<KeyboardInput>();
    else
      character->AddComponent<ControllerInput>();

    // === CHARACTER REPELLING

    // Give it a repel box trigger collider, to allow for making 2 characters slide away from each other when overlapping
    auto repelBox = character->CreateChild(CHARACTER_SLIDE_BOX_OBJECT)->AddComponent<BoxCollider>(collider, true);

    // Make it a different layer so that they collide
    repelBox->worldObject.AddComponent<CharacterRepelCollision>(body);
    repelBox->worldObject.SetPhysicsLayer(PhysicsLayer::CharacterRepelBox);

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
    auto selectedCharacter = character->AddComponent<TestCharacter>();

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

    // === HEAT DISPLAY

    // Add display in a separate child
    auto display = character->CreateChild(CHARACTER_UI_OBJECT, {0, -collider->GetBox().height / 2 - 0.2f});

    // Give it a canvas
    auto canvas = display->AddComponent<Canvas>(Canvas::Space::WorldFixedSize, Vector2{35, 50});

    // Give this UI a container
    auto container = canvas->AddChild<UIContainer>("BadgeContainer");
    container->width.Set(UIDimension::RealPixels, 35);
    container->height.Set(UIDimension::RealPixels, 50);
    container->style->textBorderSize.Set(2);
    container->style->imageScaling.Set(10);

    // TODO: add a default image size scaler to the uiObjects style as inheritable

    // Add heat text
    auto heatText = container->AddChild<UIText>("HeatDisplay", "0.0");

    // Add life icon container
    container->AddChild<UIContainer>("LifeIcons");

    // Add player indicator
    auto badgeImage = container->AddChild<UIImage>("PlayerIndicator", "./assets/sprites/badge.png");
    badgeImage->width.Set(UIDimension::RealPixels, 35);
    badgeImage->height.Set(UIDimension::RealPixels, 20);

    // Create life icon image object
    auto lifeIcon = canvas->NewUIObject<UIImage>("LifeIcon", "./assets/sprites/life.png");

    // Add a display manager
    container->AddComponent<CharacterUIManager>(fallDeath, heatText, lifeIcon);

    // // Real pixels per virtual pixel in badge UI
    // float realPerVirtualPixels{5};

    // string badgePath{"./assets/sprites/badge.png"};
    // string lifePath{"./assets/sprites/life.png"};

    // float badgeSize = Resources::GetSprite(badgePath)->GetUnscaledWidth() * realPerVirtualPixels;
    // float lifeSize = Resources::GetSprite(lifePath)->GetUnscaledWidth() * realPerVirtualPixels;

    // // Add text
    // auto badgeText = display->AddComponent<Text>("0.0", "./assets/engine/fonts/PixelOperator.ttf", 30);
    // badgeText->SetBorderSize(3);
    // badgeText->SetOffset({0, -30});
    // badgeText->SetAnchorPoint({0.5, 1});

    // // Add badge sprite
    // auto badgeSprite = display->AddComponent<SpriteRenderer>(badgePath, RenderLayer::UI);
    // badgeSprite->OverrideWidthPixels(badgeSize);
    // badgeSprite->SetAnchorPoint({0.5, 1});

    // // Add life display object manager
    // auto lifeDisplay = display->CreateChild(CHARACTER_LIFE_OBJECT, {0, -0.3f});
    // lifeDisplay->AddComponent<CharacterUIManager>(fallDeath, lifeSize, lifePath);
  };
}

auto ObjectRecipes::Platform(Vector2 size, bool withEffector) -> function<void(shared_ptr<WorldObject>)>
{
  return [size, withEffector](shared_ptr<WorldObject> platform)
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

auto ObjectRecipes::Projectile(Vector2 initialVelocity, shared_ptr<WorldObject> parent, Vector2 gravityScale) -> function<void(shared_ptr<WorldObject>)>
{
  auto weakParent{weak_ptr(parent)};
  return [initialVelocity, weakParent, gravityScale](shared_ptr<WorldObject> projectile)
  {
    projectile->SetPhysicsLayer(PhysicsLayer::Hazard);

    // Add physics
    auto body = projectile->AddComponent<Rigidbody>(RigidbodyType::Dynamic);

    // Get sprite
    projectile->AddComponent<SpriteRenderer>(RenderLayer::Projectiles);

    // Add animator
    auto animator = projectile->AddComponent<Animator>();

    // Add animations
    animator->RegisterAnimation<GeneralAnimations::Projectile>(weakParent);

    // Add speed
    body->velocity = initialVelocity;

    // Change gravity
    body->gravityScale = gravityScale;

    // Release smoke as it passes
    auto emitter = projectile->AddComponent<ParticleEmitter>(RenderLayer::VFX, 0.2, true);

    emitter->emission.color = {Color(90, 90, 90), Color(200, 200, 200)};
    emitter->emission.frequency = {0.001, 0.2};
    emitter->emission.gravityModifier = {Vector2::Up(0.1), Vector2::Down(0.01)};
    emitter->emission.lifetime = {0.2, 0.8};
    emitter->emission.speed = {0, 0};

    // If it started playing before we could set the emission params, reset it
    if (emitter->IsEmitting())
      emitter->StartEmission();
  };
}
