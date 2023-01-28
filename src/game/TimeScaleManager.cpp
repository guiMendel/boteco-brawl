#include "TimeScaleManager.h"

using namespace std;

TimeScaleManager::TimeScaleManager(WorldObject &associatedObject) : Component(associatedObject) {}

void TimeScaleManager::OnBeforeDestroy()
{
  // Reset time scales
  auto objectEntryIterator = alteredObjects.begin();
  while (objectEntryIterator != alteredObjects.end())
    objectEntryIterator = ResetTimeScale(objectEntryIterator->first);
}

void TimeScaleManager::AlterTimeScale(shared_ptr<WorldObject> target, float newScale, float duration)
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

  auto token = worldObject.DelayFunction(revert, duration);

  // Remember it
  alteredObjects.insert({targetId, token});
}

void TimeScaleManager::ResetTimeScale(std::shared_ptr<WorldObject> target)
{
  ResetTimeScale(target->id);
}

auto TimeScaleManager::ResetTimeScale(int targetId) -> decltype(alteredObjects)::iterator
{
  // Ignore unaltered objects
  if (alteredObjects.count(targetId) == 0)
    return alteredObjects.end();

  // Reset the timescale
  auto target = GetScene()->GetObject(targetId);
  if (target != nullptr)
    target->SetTimeScale(1);

  // Cancel reset if necessary
  worldObject.CancelDelayedFunction(alteredObjects[targetId]);

  // Forget it
  return alteredObjects.erase(alteredObjects.find(targetId));
}
