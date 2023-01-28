#include "GunParry.h"
#include "Character.h"

GunParry::GunParry(WorldObject &associatedObject) : Parry(associatedObject) {}

bool GunParry::CanParry(Damage damage)
{
  LOCK(damage.weakAuthor, author);

  // Can parry if is ready and attack author is a character
  return ready && author->GetComponent<Character>() != nullptr;
}

DamageParameters GunParry::Riposte(Damage damage)
{
  damage.impulse.magnitude *= 3;

  return {damage.heatDamage * 3, damage.impulse, damage.stunTime * 2};
}
