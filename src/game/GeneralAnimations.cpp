#include "GeneralAnimations.h"
#include "ParticleFX.h"
#include "Movement.h"
#include "BoxCollider.h"
#include "CircleCollider.h"
#include "Attack.h"
#include "ObjectRecipes.h"

using namespace std;
using namespace GeneralAnimations;

vector<AnimationFrame> Jump::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/jump.png", SpritesheetClipInfo(8, 8, 2), 0.1)};

  // Particles to emit on jump
  ParticleEmissionParameters particleEmission;
  particleEmission.angle = {DegreesToRadians(-135), DegreesToRadians(-100)};
  particleEmission.color = {Color::Black(), Color::Gray()};
  particleEmission.frequency = {0.0005, 0.02};
  particleEmission.lifetime = {0.2, 1.0};
  particleEmission.speed = {3, 8};
  particleEmission.gravityModifier = {Vector2::One(), Vector2::One()};

  // Add jump impulse to jump frame
  auto callback = [particleEmission](GameObject &object)
  {
    // Jump
    auto movement = object.RequireComponent<Movement>();
    movement->Jump();

    //  Don't emit if not grounded
    if (movement->IsGrounded() == false)
      return;

    // Get emission position
    auto colliderBox = object.RequireComponent<BoxCollider>()->GetBox();
    auto offset = Vector2(-colliderBox.width / 4, colliderBox.height / 2);

    // Particles
    auto emission = particleEmission;

    if (object.localScale.x < 0)
    {
      offset.x *= -1;
      emission.angle.first = M_PI - emission.angle.first;
      emission.angle.second = M_PI - emission.angle.second;
    }

    ParticleFX::EffectAt(object.GetPosition() + offset, 0.1, 0.1, emission, 3.0);
  };

  frames[1].AddCallback(callback);

  return frames;
}

vector<AnimationFrame> Neutral1::InitializeFrames()
{
  auto frames{Animation::SliceSpritesheet("./assets/sprites/punch.png", SpritesheetClipInfo(16, 8), 0.1, {4, 0})};

  // Add hitboxes
  FrameHitbox(frames[2], {Circle({7.5, 3.5}, 2), Circle({10.5, 3.5}, 2), Circle({13.5, 3.5}, 2)});
  FrameHitbox(frames[3], {Circle({10.5, 3.5}, 2), Circle({13.5, 3.5}, 2)});
  FrameHitbox(frames[4]);

  return frames;
}

vector<AnimationFrame> Neutral2::InitializeFrames()
{
  auto frames{Animation::SliceSpritesheet("./assets/sprites/kick.png", SpritesheetClipInfo(16, 8), 0.1, {4, 0})};

  // Add hitboxes
  FrameHitbox(frames[2], {Circle({8, 4}, 2.5), Circle({12, 4}, 2.5)});
  FrameHitbox(frames[3], {Circle({8, 4}, 2.5), Circle({12, 4}, 2.5)});
  FrameHitbox(frames[4]);

  return frames;
}

vector<AnimationFrame> SpecialNeutral::InitializeFrames()
{
  auto frames{SliceSpritesheet("./assets/sprites/throw.png", SpritesheetClipInfo(8, 8), 0.1)};

  auto callback = [](GameObject &object)
  {
    Vector2 initialVelocity{5, -1};

    if (object.localScale.x < 0)
      initialVelocity.x *= -1;

    object.GetState()->CreateObject(
        "Projectile",
        ObjectRecipes::Projectile(initialVelocity, object.GetShared()),
        object.GetPosition());
  };

  frames[2].AddCallback(callback);

  return frames;
}

// === HELPER FUNCTIONS DEFINITIONS

tuple<float, Vector2, float> AttackAnimation::GetAttackProperties() const
{
  return {0, Vector2{0, 0}, 0};
}

void AttackAnimation::InternalOnStart() { SetupAttack(); }
void AttackAnimation::InternalOnStop()
{
  LOCK(weakAnimator, animator);

  if (attackObjectId >= 0)
    animator->gameObject.RequireChild(attackObjectId)->RequestDestroy();
}

void AttackAnimation::SetupAttack()
{
  LOCK(weakAnimator, animator);

  // Create child
  auto attackObject = animator->gameObject.CreateChild(ATTACK_OBJECT);
  attackObject->SetPhysicsLayer(PhysicsLayer::Hitbox);

  attackObjectId = attackObject->id;

  auto [damageModifier, impulse, stunTime] = GetAttackProperties();

  // Give it the attack component
  attackObject->AddComponent<Attack>(damageModifier, impulse, stunTime);
}

void AttackAnimation::FrameHitbox(AnimationFrame &frame, vector<Circle> hitboxAreas)
{
  auto callback = [this, frame, hitboxAreas](GameObject &)
  {
    if (hitboxAreas.empty())
      RemoveHitbox();
    else
      SetHitbox(frame, hitboxAreas);
  };

  frame.AddCallback(callback);
}

void AttackAnimation::SetHitbox(const AnimationFrame &frame, vector<Circle> hitboxAreas)
{
  LOCK(weakAnimator, animator);

  auto attackObject = animator->gameObject.RequireChild(attackObjectId);

  // First, remove all colliders already there
  RemoveHitbox();

  // Get sprite renderer
  auto spriteRenderer = attackObject->GetParent()->RequireComponent<SpriteRenderer>();

  // Global position of sprite's top-left pixel, in units
  Vector2 topLeftPosition = spriteRenderer->RenderPositionFor(attackObject->GetParent()->GetPosition(), frame.GetSprite());

  // Displacement to apply to attackObject's position to get to this pixel's position
  Vector2 spriteOrigin = topLeftPosition - attackObject->GetPosition();

  // When mirrored, we want to displace with reference to top-right pixel, so sum the sprite's width
  // But also keep this offset positive so as to not double-mirror it on render
  if (GetSign(attackObject->GetScale().x) < 0)
    spriteOrigin.x = -(spriteOrigin.x + frame.GetSprite()->GetWidth());

  // Now, add each provided area as a collider
  for (auto circle : hitboxAreas)
  {
    // Convert circle's virtual pixels to units, and also make it relative to the center of the top left pixel
    circle = Circle(
        spriteOrigin + (circle.center + Vector2{0.5, 0.5}) / float(Game::defaultVirtualPixelsPerUnit),
        circle.radius / float(Game::defaultVirtualPixelsPerUnit));

    attackObject->AddComponent<CircleCollider>(circle, true);
  }
}

void AttackAnimation::RemoveHitbox()
{
  LOCK(weakAnimator, animator);

  auto attackObject = animator->gameObject.GetChild(attackObjectId);

  auto colliders = attackObject->GetComponents<Collider>();

  for (auto collider : colliders)
    attackObject->RemoveComponent(collider);
}
