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

  void OnTriggerCollisionEnter(GameObject& other) override
  {
    auto parent = weakParent.lock();

    if (parent == nullptr || other.id != parent->id)
      gameObject.RequestDestroy();
  }

private:
  // Object which generated this projectile
  std::weak_ptr<GameObject> weakParent;
};

#endif