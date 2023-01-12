#include "AnimationRecipes.h"
#include "Attack.h"
#include "CircleCollider.h"
#include "Movement.h"
#include "Animator.h"
#include "ParticleFX.h"
#include "BoxCollider.h"
#include "ObjectRecipes.h"

using namespace std;
using namespace Helper;

void AttackSetup(AnimationFrame &frame, GameObject &parent, float damageModifier, Vector2 impulse)
{
  auto weakParent = weak_ptr(parent.GetShared());

  auto callback = [weakParent, damageModifier, impulse](GameObject &)
  {
    LOCK(weakParent, parent);
    AnimationRecipes::SetupAttack(parent, damageModifier, impulse);
  };

  frame.AddCallback(callback);
}

void ResetHitbox(AnimationFrame &frame, GameObject &parent, vector<Circle> hitboxAreas = {})
{
  auto weakParent = weak_ptr(parent.GetShared());

  auto callback = [weakParent, hitboxAreas, frame](GameObject &)
  {
    LOCK(weakParent, parent);
    auto attackObject = parent->RequireChild(ATTACK_OBJECT);
    AnimationRecipes::SetHitbox(attackObject, hitboxAreas, frame);
  };

  frame.AddCallback(callback);
}

shared_ptr<GameObject> AnimationRecipes::SetupAttack(shared_ptr<GameObject> object, float damageModifier, Vector2 impulse)
{
  // Create child
  auto attackObject = object->CreateChild(ATTACK_OBJECT);
  attackObject->SetPhysicsLayer(PhysicsLayer::Hitbox);

  // Give it the attack component
  attackObject->AddComponent<Attack>(damageModifier, impulse);

  return attackObject;
}

void AnimationRecipes::SetHitbox(shared_ptr<GameObject> attackObject, vector<Circle> hitboxAreas, const AnimationFrame &frame)
{
  // First, remove all colliders already there
  RemoveHitbox(attackObject);

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

void AnimationRecipes::RemoveHitbox(shared_ptr<GameObject> attackObject)
{
  auto colliders = attackObject->GetComponents<Collider>();

  for (auto collider : colliders)
    attackObject->RemoveComponent(collider);
}

auto AnimationRecipes::Run(Animator &animator) -> shared_ptr<Animation>
{
  return make_shared<Animation>("run",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/Run.png", SpritesheetClipInfo(8, 8, 10), 0.1), true);
}

auto AnimationRecipes::Idle(Animator &animator) -> shared_ptr<Animation>
{
  return make_shared<Animation>("idle",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/idle.png", SpritesheetClipInfo(8, 8), 0.1), true);
}

auto AnimationRecipes::Jump(Animator &animator) -> shared_ptr<Animation>
{
  auto animation = make_shared<Animation>("jump",
                                          animator,
                                          Animation::SliceSpritesheet("./assets/sprites/jump.png", SpritesheetClipInfo(8, 8, 2), 0.1));

  // Particles to emit on jump
  ParticleEmissionParameters particleEmission;
  particleEmission.angle = {DegreesToRadians(-135), DegreesToRadians(-100)};
  particleEmission.color = {Color::Black(), Color::Gray()};
  particleEmission.frequency = {0.0005, 0.02};
  particleEmission.lifetime = {0.2, 1.0};
  particleEmission.speed = {3, 8};
  particleEmission.gravityModifier = {Vector2::One(), Vector2::One()};

  // Add jump impulse to jump frame
  animation->frames[1].AddCallback([particleEmission](GameObject &object)
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
                                     //
                                   });

  return animation;
}

auto AnimationRecipes::Rise(Animator &animator) -> shared_ptr<Animation>
{
  auto clipInfo = SpritesheetClipInfo(8, 8, 1);
  clipInfo.startingFrame = 2;

  return make_shared<Animation>("rise",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/jump.png", clipInfo, 0.3), true);
}

auto AnimationRecipes::Fall(Animator &animator) -> shared_ptr<Animation>
{
  auto clipInfo = SpritesheetClipInfo(8, 8, 1);
  clipInfo.startingFrame = 3;

  return make_shared<Animation>("fall",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/jump.png", clipInfo, 0.1), true);
}

auto AnimationRecipes::Land(Animator &animator) -> shared_ptr<Animation>
{
  auto clipInfo = SpritesheetClipInfo(8, 8, 1);
  clipInfo.startingFrame = 4;

  return make_shared<Animation>("land",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/jump.png", clipInfo, 0.1));
}

auto AnimationRecipes::Brake(Animator &animator) -> shared_ptr<Animation>
{
  auto clipInfo = SpritesheetClipInfo(8, 8, 1);
  clipInfo.startingFrame = 1;

  return make_shared<Animation>("brake",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/carry.png", clipInfo, 0.1), true);
}

auto AnimationRecipes::Neutral1(Animator &animator) -> shared_ptr<Animation>
{
  auto frames = Animation::SliceSpritesheet("./assets/sprites/punch.png", SpritesheetClipInfo(16, 8), 0.1, {4, 0});
  auto animation = make_shared<Animation>("neutral1", animator, frames);

  // Setup attack params
  AttackSetup(animation->frames[0], animator.gameObject, 1, Vector2::Angled(DegreesToRadians(10), 2));

  // Add hitboxes
  ResetHitbox(animation->frames[2], animator.gameObject, {Circle({7.5, 3.5}, 2), Circle({10.5, 3.5}, 2), Circle({13.5, 3.5}, 2)});
  ResetHitbox(animation->frames[3], animator.gameObject, {Circle({10.5, 3.5}, 2), Circle({13.5, 3.5}, 2)});
  ResetHitbox(animation->frames[4], animator.gameObject);

  // animation->frames[2].SetDuration(10);

  return animation;
}

auto AnimationRecipes::Neutral2(Animator &animator) -> shared_ptr<Animation>
{
  auto frames = Animation::SliceSpritesheet("./assets/sprites/kick.png", SpritesheetClipInfo(16, 8), 0.1, {4, 0});
  auto animation = make_shared<Animation>("neutral2", animator, frames);

  // Setup attack params
  AttackSetup(animation->frames[0], animator.gameObject, 1.2, Vector2::Angled(DegreesToRadians(10), 5));

  // Add hitboxes
  ResetHitbox(animation->frames[2], animator.gameObject, {Circle({8, 4}, 2.5), Circle({12, 4}, 2.5)});
  ResetHitbox(animation->frames[3], animator.gameObject, {Circle({8, 4}, 2.5), Circle({12, 4}, 2.5)});
  ResetHitbox(animation->frames[4], animator.gameObject);

  // animation->frames[2].SetDuration(10);

  return animation;
}

auto AnimationRecipes::Dash(Animator &animator) -> shared_ptr<Animation>
{
  auto clipInfo = SpritesheetClipInfo(8, 8, 3);
  clipInfo.startingFrame = 5;

  return make_shared<Animation>("dash",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/carry.png", clipInfo, 0.1));
}

auto AnimationRecipes::SpecialNeutral(Animator &animator) -> shared_ptr<Animation>
{
  auto animation = make_shared<Animation>("specialNeutral",
                                          animator,
                                          Animation::SliceSpritesheet("./assets/sprites/throw.png", SpritesheetClipInfo(8, 8), 0.1));

  animation->frames[2].AddCallback([](GameObject &object)
                                   {
                                     Vector2 initialVelocity{5, -1};

                                     if (object.localScale.x < 0)
                                       initialVelocity.x *= -1;

                                     object.GetState()->CreateObject(
                                         "Projectile",
                                         ObjectRecipes::Projectile(initialVelocity, object.GetShared()),
                                         object.GetPosition());
                                     //
                                   });

  return animation;
}