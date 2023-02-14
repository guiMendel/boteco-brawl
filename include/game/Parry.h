#ifndef __PARRY__
#define __PARRY__

#include "WorldComponent.h"
#include "Damage.h"

class Parry : public WorldComponent
{
public:
  Parry(GameObject &associatedObject);
  virtual ~Parry() {}

  // Whether a given attack damage can be parried right now
  virtual bool CanParry(Damage damage);

  // Set whether character is able to parry
  void SetParry(bool value);

  // Get the parry's ripost damage
  DamageParameters Riposte(Damage damage);

protected:
  virtual DamageParameters GetRiposteDamage(Damage damage) = 0;

  // Whether parry frames are active
  bool ready{false};
};

#endif