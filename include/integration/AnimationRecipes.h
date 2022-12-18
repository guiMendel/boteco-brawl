#ifndef __ANIMATION_RECIPES__
#define __ANIMATION_RECIPES__

#include <memory>
#include "Animation.h"

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
  static auto Punch(Animator &animator) -> std::shared_ptr<Animation>;
  static auto Dash(Animator &animator) -> std::shared_ptr<Animation>;
};

#endif