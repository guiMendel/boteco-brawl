#include "CharacterRepelCollision.h"
#include "CharacterStateRecipes.h"

using namespace std;

const float CharacterRepelCollision::maxSlideAcceleration{0.15};
const float CharacterRepelCollision::slideAccelerationDecay{0.05};

CharacterRepelCollision::CharacterRepelCollision(GameObject &associatedObject, shared_ptr<Rigidbody> body)
    : Component(associatedObject), weakBody(body), weakStateManager(body->gameObject.RequireComponent<CharacterStateManager>()) {}

void CharacterRepelCollision::OnTriggerCollision(TriggerCollisionData triggerData)
{
  if (IsEnabled() == false)
    return;

  // Also ignore if is currently attacking or dashing
  LOCK(weakStateManager, stateManager);

  if (stateManager->HasState(ATTACKING_STATE) || stateManager->HasState(DASHING_STATE))
    return;

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

  // Apply masses
  if (body->GetMass() != otherBody->GetMass())
    acceleration *= 2 * otherBody->GetMass() / (body->GetMass() + otherBody->GetMass());

  // Apply it
  body->velocity += distanceVector.Normalized() * acceleration;
}
