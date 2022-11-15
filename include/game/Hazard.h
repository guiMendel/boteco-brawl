#ifndef __HAZARD__
#define __HAZARD__

#include "Component.h"

class Hazard : public Component
{
public:
  Hazard(
      GameObject &associatedObject,
      Tag targetTag,
      float damage = 50.0f,
      bool destroyOnCollide = true);
      
  virtual ~Hazard() {}

  void OnCollision(GameObject &other) override;

  float GetDamage() { return damage; }

private:
  // Which tag of object will this projectile collide with
  Tag targetTag;

  // How much damage it will do
  float damage;

  bool destroyOnCollide;
};

#endif