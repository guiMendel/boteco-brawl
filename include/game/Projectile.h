#ifndef __PROJECTILE__
#define __PROJECTILE__

#include "GameObject.h"
#include "Component.h"

class Projectile : public Component
{
public:
  Projectile(GameObject &associatedObject, std::shared_ptr<GameObject> parent)
      : Component(associatedObject), weakParent(parent) {}

  virtual ~Projectile() {}

  void OnCollisionEnter(Collision::CollisionData data) override
  {
    auto parent = weakParent.lock();
    auto otherCollider = data.other.lock();

    if (otherCollider != nullptr && (parent == nullptr || otherCollider->gameObject.id != parent->id))
      gameObject.RequestDestroy();
  }

private:
  // Object which generated this projectile
  std::weak_ptr<GameObject> weakParent;
};

#endif