#include "AnimationRecipes.h"
#include "Animator.h"

using namespace std;

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
                                Animation::SliceSpritesheet("./assets/sprites/idle.png", SpritesheetClipInfo(8, 8), 0.1));
}

auto AnimationRecipes::Jump(Animator &animator) -> shared_ptr<Animation>
{
  return make_shared<Animation>("jump",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/jump.png", SpritesheetClipInfo(8, 8, 2), 0.1));
}

auto AnimationRecipes::Rise(Animator &animator) -> shared_ptr<Animation>
{
  auto clipInfo = SpritesheetClipInfo(8, 8, 1);
  clipInfo.startingFrame = 2;

  return make_shared<Animation>("rise",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/jump.png", clipInfo, 0.1));
}

auto AnimationRecipes::Fall(Animator &animator) -> shared_ptr<Animation>
{
  auto clipInfo = SpritesheetClipInfo(8, 8, 1);
  clipInfo.startingFrame = 3;

  return make_shared<Animation>("fall",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/jump.png", clipInfo, 0.1));
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
                                Animation::SliceSpritesheet("./assets/sprites/carry.png", clipInfo, 0.1));
}

auto AnimationRecipes::Punch(Animator &animator) -> shared_ptr<Animation>
{
  return make_shared<Animation>("punch",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/throw.png", SpritesheetClipInfo(8, 8), 0.1));
}
