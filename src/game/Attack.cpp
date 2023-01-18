#include "Attack.h"
#include "CharacterController.h"

using namespace std;

Attack::Attack(GameObject &associatedObject, DamageParameters damage, float hitSecondsCooldown)
    : Component(associatedObject), damage(damage), hitCooldown(hitSecondsCooldown * 1000) {}

void Attack::Awake()
{
  weakCharacter = gameObject.GetParent()->RequireComponent<Character>();
}

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

  // Apply hit
  targetController->TakeHit(GetDamage());

  // Register hit time
  struckTargetsTime[targetController->id] = SDL_GetTicks();
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
