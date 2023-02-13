#include "Canvas.h"
#include "Invulnerability.h"
#include "ObjectRecipes.h"
#include "Resources.h"
#include "CharacterVFX.h"
#include "ObjectRecipes.h"
#include "LandingAttackEffector.h"
#include "Arena.h"
#include "FallDeath.h"
#include "PlatformEffector.h"
#include "CharacterTest.h"
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
#include "CharacterTestAnimations.h"
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

auto ObjectRecipes::Camera() -> Recipe
{
  return [](shared_ptr<WorldObject> cameraObject)
  {
    auto camera = cameraObject->AddComponent<::Camera>();

    // Register it prematurely
    camera->RegisterToScene();
  };
}

auto ObjectRecipes::Platform(Vector2 size, bool withEffector) -> Recipe
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

auto ObjectRecipes::Projectile(Vector2 initialVelocity, shared_ptr<WorldObject> parent, Vector2 gravityScale) -> Recipe
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
    animator->RegisterAnimation<CharacterTestAnimations::Projectile>(weakParent);

    // Add speed
    body->velocity = initialVelocity;

    // Change gravity
    body->gravityScale = gravityScale;

    // Release smoke as it passes
    auto emitter = projectile->AddComponent<ParticleEmitter>(RenderLayer::VFX,
                                                             make_unique<Circle>(0.2),
                                                             true);

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

auto ObjectRecipes::Canvas(Canvas::Space space) -> Recipe
{
  return [space](shared_ptr<WorldObject> canvas)
  {
    canvas->AddComponent<::Canvas>(space);
  };
}

void ObjectRecipes::InitializeCharacter(shared_ptr<::Character> character, shared_ptr<Player> player)
{
  // Quick access to object
  auto &characterObject = character->worldObject;

  // Set it's physics layer
  characterObject.SetPhysicsLayer(PhysicsLayer::Character);

  // === RENDERING

  // Get sprite
  auto spriteRenderer = characterObject.AddComponent<SpriteRenderer>(RenderLayer::Characters);

  // Add animator
  auto animator = characterObject.AddComponent<Animator>();

  // Add animations
  character->AddAnimations(animator);

  // === COLLISION

  // Give it collision
  auto body = characterObject.AddComponent<Rigidbody>(RigidbodyType::Dynamic, 0, 0);
  auto collider = characterObject.AddComponent<BoxCollider>(character->GetHurtbox(),
                                                            false,
                                                            character->GetDensity());

  // Turn on continuous collision
  body->continuousCollisions = true;

  // === PARTICLE EFFECTS

  // Give it VFX emitters
  characterObject.AddComponent<CharacterVFX>();

  // === DYING FROM FALLING OFF

  // Give it invulnerability
  characterObject.AddComponent<Invulnerability>();

  auto fallDeath = characterObject.AddComponent<FallDeath>();

  // === MOVEMENT CONTROL

  // Give it movement
  characterObject.AddComponent<::CharacterStateManager>();
  characterObject.AddComponent<Movement>(character->GetAcceleration(),
                                         character->GetDefaultSpeed(),
                                         collider->GetBox().height / 2);

  // Give it control
  characterObject.AddComponent<CharacterController>(player);

  // Give it input
  if (player->IsMain())
    characterObject.AddComponent<KeyboardInput>();
  else
    characterObject.AddComponent<ControllerInput>();

  // === CHARACTER REPELLING

  // Give it a repel box trigger collider, to allow for making 2 characterObjects slide away from each other when overlapping
  auto repelBox = characterObject.CreateChild(CHARACTER_SLIDE_BOX_OBJECT)->AddComponent<BoxCollider>(collider, true);

  // Make it a different layer so that they collide
  repelBox->worldObject.AddComponent<CharacterRepelCollision>(body);
  repelBox->worldObject.SetPhysicsLayer(PhysicsLayer::CharacterRepelBox);

  // === PLATFORM DETECTOR FOR DROPPING

  // Give it a repel box trigger collider, to allow for making 2 characterObjects slide away from each other when overlapping
  auto platformDropDetector = characterObject.CreateChild(CHARACTER_PLATFORM_DROP_OBJECT);
  platformDropDetector->AddComponent<PlatformDrop>(body);

  // Rectangle for it's collider: a bit bigger than the regular collider
  platformDropDetector->AddComponent<BoxCollider>(collider, true, ColliderDensity::Ground, Vector2::One() * 1.2);

  // Make it a different layer so that they collide only with platforms
  platformDropDetector->SetPhysicsLayer(PhysicsLayer::CharacterPlatformDrop);

  // === ATTACKING

  // Make it vulnerable
  characterObject.AddComponent<Heat>(character->GetDefaultArmor());

  // === HEAT DISPLAY

  // Add display in a separate child
  auto display = characterObject.CreateChild(CHARACTER_UI_OBJECT, {0, -collider->GetBox().height / 2 - 0.2f});

  // Give it a canvas
  auto canvas = display->AddComponent<::Canvas>(Canvas::Space::WorldFixedSize, Vector2{100, 90});
  canvas->anchorPoint = {0.5, 1};

  // Give this UI a container
  auto container = canvas->AddChild<UIContainer>("BadgeContainer");
  container->width.Set(UIDimension::Percent, 100);
  container->height.Set(UIDimension::Percent, 100);
  container->style->textBorderSize.Set(2);
  container->style->fontSize.Set(25);
  container->style->imageScaling.Set(5);
  container->style->imageColor.Set(player->GetColor());
  container->Flexbox().mainAxis = UIDimension::Vertical;
  container->Flexbox().gap.Set(UIDimension::RealPixels, 5);
  container->Flexbox().placeItems = {0.5, 1};

  // Add heat text
  auto heatText = container->AddChild<UIText>("HeatDisplay", "0.0");

  // Add life icon container
  auto lifeContainer = container->AddChild<UIContainer>(CHARACTER_LIFE_OBJECT);
  lifeContainer->Flexbox().gap.Set(UIDimension::RealPixels, 6);
  lifeContainer->Flexbox().placeItems = {0.5, 0.5};
  weak_ptr<UIContainer> weakLifeContainer = lifeContainer;

  // Add player indicator
  auto badgeImage = container->AddChild<UIImage>("PlayerIndicator", "./assets/sprites/badge.png");

  // Creates life icon image object and adds it to it's container
  auto addLifeIcon = [weakLifeContainer]()
  {
    auto lifeIcon = Lock(weakLifeContainer)->AddChild<UIImage>("LifeIcon", "./assets/sprites/life.png");
    // lifeIcon->margin.Set(UIDimension::RealPixels, 5);
  };

  // Add a display manager
  container->AddComponent<CharacterUIManager>(fallDeath, heatText, addLifeIcon);

  // === SPECIFIC MECHANICS

  // Add in this character's specific mechanics
  character->AddMechanics(characterObject.GetShared());
}
