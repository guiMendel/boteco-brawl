#ifndef __TRIGGER_COLLISION_DATA__
#define __TRIGGER_COLLISION_DATA__

#include <memory>

class Collider;

struct TriggerCollisionData
{
  std::weak_ptr<Collider> weakSource;

  std::weak_ptr<Collider> weakOther;

  size_t GetHash() const;
};

#endif