#include "LandingAttackEffector.h"
#include "Actions.h"

using namespace std;

LandingAttackEffector::LandingAttackEffector(GameObject &associatedObject, function<bool()> effectorCondition)
    : LandEffector(associatedObject), effectorCondition(effectorCondition) {}

shared_ptr<Action> LandingAttackEffector::GetLandAction()
{
  if (effectorCondition()) {
    auto body = worldObject.RequireComponent<Rigidbody>();
    return make_shared<Actions::LandingAttack>(body->velocity.Magnitude());
  }

  else
    return nullptr;
}
