#ifndef __PARRY__
#define __PARRY__

#include "Component.h"
#include "Damage.h"

class Parry : public Component
{
public:
  Parry(GameObject &associatedObject) : Component(associatedObject) {}
  virtual ~Parry() {}

  // Whether a given attack damage can be parried right now
  virtual bool CanParry(Damage damage) = 0;

  // Get the parry's ripost damage
  virtual DamageParameters Riposte(Damage damage) = 0;
};

#endif