#ifndef __GUN_PARRY__
#define __GUN_PARRY__

#include "Parry.h"

class GunParry : public Parry
{
public:
  GunParry(WorldObject &associatedObject);
  virtual ~GunParry() {}

  // Whether a given attack damage can be parried right now
  bool CanParry(Damage damage) override;

  // Trigger this parry's riposte for a given attack damage
  DamageParameters Riposte(Damage damage) override;

  // Whether parry frames are active
  bool ready{false};
};

#endif