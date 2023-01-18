#include "Damage.h"
#include "Helper.h"
#include "GameObject.h"

using namespace std;
using namespace Helper;

AttackImpulse::AttackImpulse()
    : magnitude(0), type(Directed), direction(Vector2::Zero()) {}

AttackImpulse::AttackImpulse(Vector2 direction, float magnitude)
    : magnitude(magnitude), type(Directed), direction(direction.Normalized()) {}

AttackImpulse::AttackImpulse(std::shared_ptr<GameObject> origin, float magnitude)
    : magnitude(magnitude), type(Repulsion), weakOrigin(origin) {}

Vector2 AttackImpulse::DeriveImpulse(std::shared_ptr<GameObject> target) const
{
  switch (type)
  {
  case Directed:
    return direction * magnitude * Vector2(mirrored ? -1 : 1, 1);

  case Repulsion:
    LOCK(weakOrigin, origin);
    return (target->GetPosition() - origin->GetPosition()).Normalized() * magnitude;
  }

  throw runtime_error("Trying to derive impulse with unrecognized type");
}

Damage DamageParameters::DeriveDamage(float baseHeatDamage, shared_ptr<GameObject> author) const
{
  AttackImpulse damageImpulse{impulse};
  damageImpulse.mirrored = author->GetScale().x < 0;

  return Damage{
      // Total damage
      baseHeatDamage * heatMultiplier,
      // Impulse
      damageImpulse,
      // Stun time
      stunTime,
      // Attack author
      author};
}
