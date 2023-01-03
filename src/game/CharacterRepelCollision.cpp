#include "CharacterRepelCollision.h"

using namespace std;

const float CharacterRepelCollision::maxSlideAcceleration{0.2};
const float CharacterRepelCollision::slideAccelerationDecay{0.05};

CharacterRepelCollision::CharacterRepelCollision(GameObject &associatedObject, shared_ptr<Rigidbody> body)
    : Component(associatedObject), weakBody(body) {}

void CharacterRepelCollision::OnTriggerCollision(TriggerCollisionData triggerData)
{
  LOCK(triggerData.weakOther, other);
  LOCK(other->rigidbodyWeak, otherBody);

  // Slide away from it
  SlideAwayFrom(otherBody);
}

void CharacterRepelCollision::SlideAwayFrom(std::shared_ptr<Rigidbody> otherBody)
{
  // Get our own body
  LOCK(weakBody, body);

  // Get distance
  auto distanceVector = gameObject.GetPosition() - otherBody->gameObject.GetPosition();

  // Ignore y axis
  distanceVector.y = 0;

  // Get acceleration to apply
  float acceleration = max(maxSlideAcceleration - slideAccelerationDecay * distanceVector.Magnitude(), 0.0f);

  // Apply it
  body->velocity += distanceVector.Normalized() * acceleration;
}
