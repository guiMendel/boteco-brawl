#include "Attack.h"
#include "CharacterController.h"

using namespace std;

Attack::Attack(GameObject &associatedObject, DamageParameters damage)
    : Component(associatedObject), damage(damage) {}

void Attack::Awake()
{
  weakCharacter = gameObject.GetParent()->RequireComponent<Character>();
}

void Attack::Land(shared_ptr<CharacterController> targetController)
{
  // Ignore if already hit this target
  if (struckControllerIds.count(targetController->id) > 0)
    return;

  targetController->TakeHit(GetDamage());

  struckControllerIds.insert(targetController->id);
}

void Attack::OnTriggerCollisionEnter(TriggerCollisionData trigger)
{
  LOCK(trigger.weakOther, other);

  // If other has a heat
  auto controller = other->GetOwner()->GetComponent<CharacterController>();

  // Land an attack
  if (controller != nullptr)
    Land(controller);
}

Damage Attack::GetDamage() const
{
  LOCK(weakCharacter, character);

  return damage.DeriveDamage(character->GetBaseDamage(), gameObject.GetParent());
}
