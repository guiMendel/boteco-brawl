#ifndef __LAND_EFFECTOR__
#define __LAND_EFFECTOR__

#include "WorldComponent.h"
#include "Action.h"

// Allows for modifying which action is dispatched on land
class LandEffector : public WorldComponent
{
public:
  LandEffector(GameObject &associatedObject) : WorldComponent(associatedObject) {}
  virtual ~LandEffector() {}

  // Which action to perform for landing
  // nullptr indicates the default one
  virtual std::shared_ptr<Action> GetLandAction() = 0;
};

#endif