#include "Projectile.h"
#include <cmath>
#include <string>

using namespace std;

Projectile::Projectile(
    GameObject &associatedObject,
    float startingAngle,
    float speed,
    float timeToLive,
    weak_ptr<GameObject> target,
    float chaseSteering)
    : Component(associatedObject), speed(Vector2::Angled(startingAngle, speed)), angle(startingAngle), timeToLive(timeToLive), targetWeak(target), chaseSteering(chaseSteering)
{
  // Adjust rotation
  gameObject.SetRotation(this->speed.Angle());
}

void Projectile::Update(float deltaTime)
{
  // Check if timer is up
  if ((timeToLive -= deltaTime) <= 0)
  {
    gameObject.RequestDestroy();

    return;
  }

  // Chase
  Chase();

  // Move
  gameObject.localPosition += speed * deltaTime;
}

void Projectile::Chase()
{
  // Get target
  auto target = targetWeak.lock();

  if (!target)
    return;

  // Get angle between projectile and target
  float targetSteer = Vector2::AngleBetween(gameObject.GetPosition(), target->GetPosition()) - angle;

  // Get angle to steer
  float steerAngle = std::abs(targetSteer) > std::abs(chaseSteering) ? chaseSteering : targetSteer;

  // Steer
  speed = speed.Rotated(steerAngle);

  // Adjust rotation
  gameObject.SetRotation(speed.Angle());
}
