#include "CharacterSlideCollision.h"

using namespace std;

const float CharacterSlideCollision::maxSlideAcceleration{0.4};
const float CharacterSlideCollision::slideAccelerationDecay{0.2};

CharacterSlideCollision::CharacterSlideCollision(GameObject &associatedObject, shared_ptr<Rigidbody> body)
    : Component(associatedObject), weakBody(body) {}

void CharacterSlideCollision::OnTriggerCollision(GameObject &other)
{
  cout << gameObject.GetName() << " sliding with " << other.GetName() << endl;

  // Slide away from it
  SlideAwayFrom(other.RequireComponent<Rigidbody>());
}

void CharacterSlideCollision::SlideAwayFrom(std::shared_ptr<Rigidbody> otherBody)
{
  // Get our own body
  LOCK(weakBody, body);

  // Get distance
  auto distanceVector = gameObject.GetPosition() - otherBody->gameObject.GetPosition();

  // Get acceleration to apply
  float acceleration = max(maxSlideAcceleration - slideAccelerationDecay * distanceVector.Magnitude(), 0.0f);

  // Apply it
  body->velocity += distanceVector.Normalized() * acceleration;
}
