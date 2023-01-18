#include "TimeScaleManager.h"

using namespace std;

TimeScaleManager::TimeScaleManager(GameObject &associatedObject) : Component(associatedObject) {}

void TimeScaleManager::AlterTimeScale(shared_ptr<GameObject> target, float newScale, float duration)
{
  Assert(newScale > 0, "Invalid new time scale: it must be a positive value");

  // First reset it if it's already altered
  ResetTimeScale(target);

  if (duration == 0)
    return;

  // Alter it's timescale
  target->SetTimeScale(newScale);

  // Store it's id
  int targetId = target->id;

  // Revert it after a while
  auto revert = [this, targetId]()
  {
    ResetTimeScale(targetId);
  };

  auto token = gameObject.DelayFunction(revert, duration);

  // Remember it
  alteredObjects.insert({targetId, token});
}

void TimeScaleManager::ResetTimeScale(std::shared_ptr<GameObject> target)
{
  ResetTimeScale(target->id);
}

void TimeScaleManager::ResetTimeScale(int targetId)
{
  // Ignore unaltered objects
  if (alteredObjects.count(targetId) == 0)
    return;

  // Reset the timescale
  GetState()->RequireObject(targetId)->SetTimeScale(1);

  cout << *GetState()->RequireObject(targetId) << " timescale reset back to " << GetState()->RequireObject(targetId)->GetTimeScale() << endl;

  // Cancel reset if necessary
  gameObject.CancelDelayedFunction(alteredObjects[targetId]);

  // Forget it
  alteredObjects.erase(targetId);
}
