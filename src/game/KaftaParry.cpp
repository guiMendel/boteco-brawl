#include "KaftaParry.h"
#include "Character.h"

KaftaParry::KaftaParry(GameObject &associatedObject) : Parry(associatedObject) {}

bool KaftaParry::CanParry(Damage damage)
{
  // Can parry if is ready and attack author is a character
  return Lock(damage.weakAuthor)->GetComponent<Character>() != nullptr && Parry::CanParry(damage);
}

DamageParameters KaftaParry::GetRiposteDamage(Damage damage)
{
  damage.impulse.magnitude *= 2.2;

  return {damage.heatDamage * 3, damage.impulse, damage.stunTime * 2};
}
