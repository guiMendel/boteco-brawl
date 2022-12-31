#ifndef __CHARACTER_SLIDE_COLLISION__
#define __CHARACTER_SLIDE_COLLISION__

#include "GameObject.h"
#include "Component.h"
#include "Rigidbody.h"

class CharacterSlideCollision : public Component
{
public:
  CharacterSlideCollision(GameObject &associatedObject, std::shared_ptr<Rigidbody> body);
  virtual ~CharacterSlideCollision() {}

  void OnTriggerCollision(GameObject &) override;

private:
  // Max acceleration for sliding away, units/s/s
  static const float maxSlideAcceleration;

  // How much acceleration decays as distance between bodies increases, in units/s/s/unit
  static const float slideAccelerationDecay;

  // Adds velocity to slide away from the given body
  void SlideAwayFrom(std::shared_ptr<Rigidbody> otherBody);

  // Reference to character's body
  std::weak_ptr<Rigidbody> weakBody;
};

#endif