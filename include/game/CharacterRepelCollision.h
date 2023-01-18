#ifndef __CHARACTER_SLIDE_COLLISION__
#define __CHARACTER_SLIDE_COLLISION__

#include "GameObject.h"
#include "Component.h"
#include "Rigidbody.h"
#include "CharacterStateManager.h"

class CharacterRepelCollision : public Component
{
public:
  CharacterRepelCollision(GameObject &associatedObject, std::shared_ptr<Rigidbody> body);
  virtual ~CharacterRepelCollision() {}

  void OnTriggerCollision(TriggerCollisionData triggerData) override;

private:
  // Max acceleration for sliding away, units/s/s
  static const float maxSlideAcceleration;

  // How much acceleration decays as distance between bodies increases, in units/s/s/unit
  static const float slideAccelerationDecay;

  // Adds velocity away from the given body
  void SlideAwayFrom(std::shared_ptr<Rigidbody> otherBody);

  // Reference to character's body
  std::weak_ptr<Rigidbody> weakBody;
  std::weak_ptr<CharacterStateManager> weakStateManager;
};

#endif