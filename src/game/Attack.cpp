#include "Attack.h"
#include "Heat.h"

using namespace std;

Attack::Attack(GameObject &associatedObject, float damageModifier, Vector2 impulse)
    : Component(associatedObject), damageModifier(damageModifier), impulse(impulse) {}

void Attack::Awake()
{
  weakStateManager = gameObject.RequireComponent<CharacterStateManager>();
}

void Attack::Land(shared_ptr<Heat> targetHeat)
{
  // Ignore if already hit this target
  if (struckHeatIds.count(targetHeat->id) > 0)
    return;

  targetHeat->TakeDamage(GetDamage());

  struckHeatIds.insert(targetHeat->id);
}

void Attack::OnTriggerCollisionEnter(TriggerCollisionData trigger)
{
  LOCK(trigger.weakOther, other);

  // If other has a heat
  auto heat = other->GetOwner()->GetComponent<Heat>();

  // Land an attack
  if (heat != nullptr)
    Land(heat);
}

Damage Attack::GetDamage() const
{
  LOCK(weakStateManager, character);

  return Damage{character->baseDamage * damageModifier, impulse};
}
