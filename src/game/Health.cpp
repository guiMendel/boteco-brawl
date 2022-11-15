#include "Health.h"

Health::Health(GameObject &associatedObject, float totalHealth, bool destroyOnDeath)
    : Component(associatedObject), healthPoints(totalHealth), destroyOnDeath(destroyOnDeath) {}

void Health::TakeDamage(float damage)
{
  healthPoints -= damage;

  if (healthPoints <= 0 && deathTriggered == false)
  {
    OnDeath.Invoke();
    deathTriggered = true;

    // Destroy if necessary
    if (destroyOnDeath)
      gameObject.RequestDestroy();
  }
}