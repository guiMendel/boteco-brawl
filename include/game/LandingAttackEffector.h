#ifndef __LANDING_ATTACK_EFFECTOR__
#define __LANDING_ATTACK_EFFECTOR__

#include "LandEffector.h"
#include <functional>

// Allows for modifying which action is dispatched on land
class LandingAttackEffector : public LandEffector
{
public:
  LandingAttackEffector(GameObject &associatedObject, std::function<bool()> effectorCondition);
  virtual ~LandingAttackEffector() {}

  // Which action to perform for landing
  // nullptr indicates the default one
  std::shared_ptr<Action> GetLandAction() override;

private:
  // Whether, at the time of it's call, the effector should override the land action
  std::function<bool()> effectorCondition;
};

#endif