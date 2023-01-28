#include "TriggerCollisionData.h"
#include "Collider.h"

size_t TriggerCollisionData::GetHash() const
{
  auto source = weakSource.lock();
  auto other = weakOther.lock();

  // Ensure valid pointers
  Helper::Assert(source != nullptr && other != nullptr, "Failed to lock collision data's colliders");
  return Helper::HashTwo(source->id, other->id);
}