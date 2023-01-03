#include "AnimationRecipes.h"
#include "Movement.h"
#include "Animator.h"
#include "ParticleFX.h"
#include "ObjectRecipes.h"

using namespace std;
using namespace Helper;

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
                                     auto colliderBox = object.RequireComponent<Collider>()->GetBox();
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

auto AnimationRecipes::Punch(Animator &animator) -> shared_ptr<Animation>
{
  return make_shared<Animation>("punch",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/throw.png", SpritesheetClipInfo(8, 8), 0.1));
}

auto AnimationRecipes::Dash(Animator &animator) -> shared_ptr<Animation>
{
  auto clipInfo = SpritesheetClipInfo(8, 8, 3);
  clipInfo.startingFrame = 5;

  return make_shared<Animation>("dash",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/carry.png", clipInfo, 0.1));
}

auto AnimationRecipes::Special(Animator &animator) -> shared_ptr<Animation>
{
  auto animation = make_shared<Animation>("special",
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