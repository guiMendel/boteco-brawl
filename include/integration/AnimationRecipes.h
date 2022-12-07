#ifndef __ANIMATION_RECIPES__
#define __ANIMATION_RECIPES__

#include <memory>
#include "Animation.h"

class Animator;

class AnimationRecipes
{
public:
  static auto Run(Animator &animator) -> std::shared_ptr<Animation>;
};

#endif