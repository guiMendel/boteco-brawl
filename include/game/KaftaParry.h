#ifndef __KAFTA_PARRY__
#define __KAFTA_PARRY__

#include "Parry.h"

class KaftaParry : public Parry
{
public:
  KaftaParry(GameObject &associatedObject);
  virtual ~KaftaParry() {}

  // Whether a given attack damage can be parried right now
  bool CanParry(Damage damage) override;

  // Trigger this parry's riposte for a given attack damage
  DamageParameters GetRiposteDamage(Damage damage) override;
};

#endif