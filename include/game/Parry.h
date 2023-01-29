#ifndef __PARRY__
#define __PARRY__

#include "WorldComponent.h"
#include "Damage.h"

class Parry : public WorldComponent
{
public:
  Parry(GameObject &associatedObject) : WorldComponent(associatedObject) {}
  virtual ~Parry() {}

  // Whether a given attack damage can be parried right now
  virtual bool CanParry(Damage damage) = 0;

  // Get the parry's ripost damage
  virtual DamageParameters Riposte(Damage damage) = 0;
};

#endif