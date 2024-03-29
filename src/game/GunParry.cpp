#include "GunParry.h"
#include "Character.h"

GunParry::GunParry(GameObject &associatedObject) : Parry(associatedObject) {}

bool GunParry::CanParry(Damage damage)
{
  // Can parry if is ready and attack author is a character
  return Lock(damage.weakAuthor)->GetComponent<Character>() != nullptr && Parry::CanParry(damage);
}

DamageParameters GunParry::GetRiposteDamage(Damage damage)
{
  damage.impulse.magnitude *= 3;

  return {damage.heatDamage * 3, damage.impulse, damage.stunTime * 2};
}
