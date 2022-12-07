#include "AnimationRecipes.h"
#include "Animator.h"

using namespace std;

auto AnimationRecipes::Run(Animator &animator) -> shared_ptr<Animation>
{
  return make_shared<Animation>("run",
                                animator,
                                Animation::SliceSpritesheet("./assets/sprites/Run.png", SpritesheetClipInfo(8, 8, 10), 0.1));
}
