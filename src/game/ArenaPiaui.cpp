#include "ArenaPiaui.h"
#include "BoxCollider.h"
#include "Attack.h"
#include "ParticleEmitter.h"
#include "PlatformEffector.h"
#include "CameraBehavior.h"
#include "ObjectRecipes.h"
#include "SpriteRenderer.h"

using namespace std;

ArenaPiaui::ArenaPiaui(GameObject &associatedObject) : Arena(associatedObject) {}

void ArenaPiaui::InitializeArena()
{
  // Add platforms sprite
  auto platformsRenderer = worldObject.AddComponent<SpriteRenderer>(
      "./assets/sprites/piaui/platforms.png",
      RenderLayer::Platforms);

  // Set dimensions according to this size
  width = platformsRenderer->GetSprite()->GetWidth();
  height = platformsRenderer->GetSprite()->GetHeight();

  // Get max camera size
  float maxCameraSize = GetScene()->RequireFindComponent<CameraBehavior>()->GetMaxCameraSize();

  // Add background
  auto background = worldObject.AddComponent<SpriteRenderer>(
      "./assets/sprites/piaui/background.png",
      RenderLayer::Background);

  // Give background a parallax
  background->SetParallax(0.5, maxCameraSize);

  // SAFENET
  // GetScene()->Instantiate("safenet", ObjectRecipes::Platform({20, 2}), Vector2{8, 0})->SetParent(worldObject.GetShared());

  // Ground platforms
  vector groundPlatforms{
      Rectangle(Rectangle::TopLeftInitialize, {162.5, 209.5}, 310, 18),
      Rectangle(Rectangle::TopLeftInitialize, {-75, 246}, 201, 6),
      Rectangle(Rectangle::TopLeftInitialize, {310, 192}, 39, 18),
  };

  // Pass-through platforms
  vector passablePlatforms{
      Rectangle(Rectangle::TopLeftInitialize, {180, 117}, 48, 2),
      Rectangle(Rectangle::TopLeftInitialize, {175, 155}, 24, 4),
      Rectangle(Rectangle::TopLeftInitialize, {209, 169}, 24, 4),
      Rectangle(Rectangle::TopLeftInitialize, {376, 170}, 65, 5),
      Rectangle(Rectangle::TopLeftInitialize, {-75, 205}, 159, 4),
      Rectangle(Rectangle::TopLeftInitialize, {-75, 165}, 159, 4),
      Rectangle(Rectangle::TopLeftInitialize, {-75, 123}, 159, 4),
  };

  // Instantiate these platforms
  for (auto platform : groundPlatforms)
  {
    // Get dimensions in units
    Vector2 dimensions = Vector2{platform.width, platform.height} / float(Game::defaultVirtualPixelsPerUnit);

    // Get position in units
    Vector2 position = platformsRenderer->GetVirtualPixelOffset(platform.center);

    GetScene()->Instantiate("SolidPlatform", ObjectRecipes::Platform(dimensions), position)->SetParent(worldObject.GetShared());
  }

  for (auto platform : passablePlatforms)
  {
    // Get dimensions in units
    Vector2 dimensions = Vector2{platform.width, platform.height} / float(Game::defaultVirtualPixelsPerUnit);

    // Get position in units
    Vector2 position = platformsRenderer->GetVirtualPixelOffset(platform.center);

    GetScene()->Instantiate("PassablePlatform", ObjectRecipes::Platform(dimensions, true), position)->SetParent(worldObject.GetShared());
  }

  // Add barbecue particles
  Rectangle barbecueBox{Rectangle::TopLeftInitialize,
                        platformsRenderer->GetVirtualPixelOffset({0, 246}),
                        84 / float(Game::defaultVirtualPixelsPerUnit),
                        1 / float(Game::defaultVirtualPixelsPerUnit)};

  auto barbecueParticles = worldObject.AddComponent<ParticleEmitter>(RenderLayer::VFX, make_unique<Rectangle>(barbecueBox), true);
  barbecueParticles->emission.color = {Color{238, 198, 43}, Color{247, 70, 0}};
  barbecueParticles->emission.angle = {DegreesToRadians(-45), DegreesToRadians(-180 + 45)};
  barbecueParticles->emission.frequency = {0.1, 0.05};
  barbecueParticles->emission.lifetime = {0.4, 2};
  barbecueParticles->emission.speed = {3, 8};
  barbecueParticles->emission.gravityModifier = {Vector2::Down(0.1), Vector2::Down(0.2)};

  // Add hitbox
  Rectangle barbecueHitbox{Rectangle::TopLeftInitialize,
                           platformsRenderer->GetVirtualPixelOffset({-75, 246}),
                           (84 + 75) / float(Game::defaultVirtualPixelsPerUnit),
                           1 / float(Game::defaultVirtualPixelsPerUnit)};

  auto hitboxObject = worldObject.CreateChild("BarbecueHitbox");
  hitboxObject->AddComponent<BoxCollider>(barbecueHitbox, true);
  hitboxObject->AddComponent<Attack>(DamageParameters{5, AttackImpulse{Vector2::Up(), 10}, 0.1}, 0.05);
}
