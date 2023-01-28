#ifndef __LAND_EFFECTOR__
#define __LAND_EFFECTOR__

#include "Component.h"
#include "Action.h"

// Allows for modifying which action is dispatched on land
class LandEffector : public Component
{
public:
  LandEffector(WorldObject &associatedObject) : Component(associatedObject) {}
  virtual ~LandEffector() {}

  // Which action to perform for landing
  // nullptr indicates the default one
  virtual std::shared_ptr<Action> GetLandAction() = 0;
};

#endif