#ifndef __TIME_SCALE_MANAGER__
#define __TIME_SCALE_MANAGER__

#include "Component.h"

class TimeScaleManager : public Component
{
public:
  TimeScaleManager(WorldObject &associatedObject);
  virtual ~TimeScaleManager() {}

  void OnBeforeDestroy() override;

  // Set the timescale for a given object, and revert it after the given interval
  void AlterTimeScale(std::shared_ptr<WorldObject> target, float newScale, float duration);

  // Remove any previous time scale alterations on this object
  void ResetTimeScale(std::shared_ptr<WorldObject> target);

private:
  // Remember for which objects time scale was altered, by their ids
  // Maps ids to delayed function tokens
  std::unordered_map<int, int> alteredObjects;

  auto ResetTimeScale(int objectId) -> decltype(alteredObjects)::iterator;
};

#endif