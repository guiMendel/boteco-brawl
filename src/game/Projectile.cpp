#include "Projectile.h"

using namespace std;

Projectile::Projectile(GameObject &associatedObject, shared_ptr<GameObject> parent)
    : Component(associatedObject), weakParent(parent) {}

void Projectile::OnTriggerCollisionEnter(TriggerCollisionData triggerData)
{
  auto parent = weakParent.lock();
  LOCK(triggerData.weakOther, other);

  cout << "Projectile triggered against " << other->gameObject << ", and is " << triggerData.weakSource.lock()->gameObject << endl;

  if (parent == nullptr || other->gameObject.id != parent->id)
    gameObject.RequestDestroy();
}
