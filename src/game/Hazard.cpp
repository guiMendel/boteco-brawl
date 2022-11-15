#include "Hazard.h"
#include "Health.h"

Hazard::Hazard(
    GameObject &associatedObject,
    Tag targetTag,
    float damage,
    bool destroyOnCollide)
    : Component(associatedObject), targetTag(targetTag), damage(damage), destroyOnCollide(destroyOnCollide) {}

void Hazard::OnCollision(GameObject &other)
{
  // Ignore if other isn't of target tag
  if (other.tag != targetTag)
    return;

  // Get other's health component
  auto health = other.GetComponent<Health>();

  // If it has none, stop
  if (!health)
    return;

  // Deal damage
  health->TakeDamage(damage);

  // Die
  if (destroyOnCollide)
    gameObject.RequestDestroy();
}
