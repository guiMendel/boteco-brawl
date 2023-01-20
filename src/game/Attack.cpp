#include "Attack.h"
#include "CharacterController.h"

using namespace std;

Attack::Attack(GameObject &associatedObject, DamageParameters damage, float hitSecondsCooldown)
    : Component(associatedObject), damage(damage), hitCooldown(hitSecondsCooldown * 1000) {}

void Attack::Land(shared_ptr<CharacterController> targetController)
{
  // If already hit this target
  if (struckTargetsTime.count(targetController->id) > 0)
  {
    auto elapsedTime = SDL_GetTicks() - struckTargetsTime[targetController->id];

    // Check if cooldown is elapsed
    if (hitCooldown < 0 || elapsedTime < hitCooldown)
      return;
  }

  OnConnect.Invoke(targetController);

  // Apply hit
  targetController->TakeHit(GetDamage());

  // Register hit time
  struckTargetsTime[targetController->id] = SDL_GetTicks();
}

void Attack::OnTriggerCollisionEnter(TriggerCollisionData trigger)
{
  LOCK(trigger.weakOther, other);

  // Get other object
  auto otherObject = other->GetOwner();

  // Check if ignored
  if (ignoredObjects.count(otherObject->id) > 0)
    return;

  // If other has a character controller
  auto controller = otherObject->GetComponent<CharacterController>();

  // Land an attack
  if (controller != nullptr)
    Land(controller);
}

Damage Attack::GetDamage() const
{
  return damage.DeriveDamage(gameObject.GetParent());
}

void Attack::Ignore(std::shared_ptr<GameObject> target)
{
  ignoredObjects.insert(target->id);
}
