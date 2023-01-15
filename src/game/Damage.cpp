#include "Damage.h"
#include "Helper.h"
#include "GameObject.h"

using namespace std;
using namespace Helper;

Damage DamageParameters::DeriveDamage(float baseHeatDamage, shared_ptr<GameObject> author) const
{
  float direction = GetSign(author->GetScale().x);

  return Damage{
      // Total damage
      baseHeatDamage * heatMultiplier,
      // Impulse
      impulse * Vector2(direction, 1),
      // Stun time
      stunTime,
      // Attack author
      author};
}
