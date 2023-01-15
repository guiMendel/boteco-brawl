#include "GunParry.h"
#include "Character.h"

GunParry::GunParry(GameObject &associatedObject) : Parry(associatedObject) {}

bool GunParry::CanParry(Damage damage)
{
  LOCK(damage.weakAuthor, author);

  // Can parry if is ready and attack author is a character
  return ready && author->GetComponent<Character>() != nullptr;
}

DamageParameters GunParry::Riposte(Damage damage)
{
  return {damage.heatDamage * 3, damage.impulse * 3, damage.stunTime * 2};
}
