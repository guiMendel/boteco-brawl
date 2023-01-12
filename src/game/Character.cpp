#include "Character.h"

using namespace std;

Character::Character(GameObject &associatedObject, float baseDamage)
    : Component(associatedObject), baseDamage(baseDamage) {}

int Character::TransformSequenceIndexFor(string animation, int index) const
{
  auto &transformer = GetSequenceIndexTransformer();

  // If no transformer for this animation, perform no transformation
  if (transformer.count(animation) == 0)
    return index;

  return transformer.at(animation)(index);
}

Character::indexTransformer SequenceIndexTransformer::Repeat(int animations)
{
  return [animations](int currentIndex)
  {
    return currentIndex % animations;
  };
}

Character::indexTransformer SequenceIndexTransformer::LoopSingleAfter(int loopAnimation)
{
  return [loopAnimation](int currentIndex)
  {
    return currentIndex > loopAnimation ? loopAnimation : currentIndex;
  };
}
