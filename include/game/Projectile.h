#ifndef __PROJECTILE__
#define __PROJECTILE__

#include "GameObject.h"
#include "Component.h"

class Projectile : public Component
{
public:
  Projectile(GameObject &associatedObject, std::shared_ptr<GameObject> parent);

  virtual ~Projectile() {}

  void OnTriggerCollisionEnter(TriggerCollisionData triggerData) override;

private:
  // Object which generated this projectile
  std::weak_ptr<GameObject> weakParent;
};

#endif