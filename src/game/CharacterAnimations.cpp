#include "CharacterAnimations.h"

using namespace std;

void CharacterAnimationHelper::SplitLastFrame(vector<AnimationFrame> &frames, int numberOfInstances, float newDuration)
{
  Assert(frames.size() > 0, "Can't replicate frames of empty sequence");
  Assert(numberOfInstances > 0, "Can't remove frame");

  auto &lastFrame = frames[frames.size() - 1];

  // Set new duration
  lastFrame.SetDuration(newDuration);

  // Replicate it
  frames.insert(frames.end(), numberOfInstances - 1, lastFrame);
}
