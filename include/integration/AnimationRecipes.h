#ifndef __ANIMATION_RECIPES__
#define __ANIMATION_RECIPES__

#include <memory>
#include "Animation.h"
#include "Circle.h"

class Animator;

class AnimationRecipes
{
public:
  static auto Run(Animator &animator) -> std::shared_ptr<Animation>;
  static auto Jump(Animator &animator) -> std::shared_ptr<Animation>;
  static auto Rise(Animator &animator) -> std::shared_ptr<Animation>;
  static auto Fall(Animator &animator) -> std::shared_ptr<Animation>;
  static auto Land(Animator &animator) -> std::shared_ptr<Animation>;
  static auto Idle(Animator &animator) -> std::shared_ptr<Animation>;
  static auto Brake(Animator &animator) -> std::shared_ptr<Animation>;
  static auto Neutral1(Animator &animator) -> std::shared_ptr<Animation>;
  static auto Neutral2(Animator &animator) -> std::shared_ptr<Animation>;
  static auto Dash(Animator &animator) -> std::shared_ptr<Animation>;
  static auto SpecialNeutral(Animator &animator) -> std::shared_ptr<Animation>;

  // Creates a child object that contains attack information
  static std::shared_ptr<GameObject> SetupAttack(std::shared_ptr<GameObject> object, float damageModifier, Vector2 impulse);

  // Replace any previous colliders in this object with the ones specified
  // The coordinates and radius in the circles are to be provided in virtual pixels, where {0, 0} is the sprite's top-left pixel
  static void SetHitbox(std::shared_ptr<GameObject> attackObject, std::vector<Circle> hitboxAreas, const AnimationFrame &frame);

  // Remove all colliders in this object
  static void RemoveHitbox(std::shared_ptr<GameObject> attackObject);
};

#endif