#include "Movement.h"

using namespace std;

void Movement::Update(float deltaTime)
{
  // Follow a target if it exists
  if (followTarget)
    FollowTarget(deltaTime);

  // Adjust speed
  Accelerate(deltaTime);

  // Move based on speed
  if (!velocity)
    return;

  gameObject.localPosition += velocity * deltaTime;
}

void Movement::Accelerate(float deltaTime)
{
  // Target velocity
  Vector2 targetVelocity = targetDirection * targetSpeed;

  float frameAcceleration = deltaTime * acceleration;

  // New velocity
  velocity = targetVelocity * frameAcceleration + velocity * max(1.0f - frameAcceleration, 0.0f);
}

void Movement::Move(Vector2 direction, bool cancelFollow)
{
  targetDirection = direction;
  followTarget = followTarget && !cancelFollow;
}

void Movement::MoveTo(Vector2 position, std::function<void()> callback)
{
  targetPosition = position;
  followTarget = true;
  targetReachCallback = callback;
}

void Movement::FollowTarget(float deltaTime)
{
  if (followTarget == false)
    return;

  float squareDistance = Vector2::SqrDistance(gameObject.GetPosition(), targetPosition);

  // Check if arrived
  if (squareDistance < deltaTime * deltaTime * velocity.SqrMagnitude() || squareDistance < 0.01f)
  {
    followTarget = false;
    gameObject.SetPosition(targetPosition);
    velocity = Vector2::Zero();
    targetDirection = Vector2::Zero();

    if (targetReachCallback != nullptr)
      targetReachCallback();

    return;
  }

  // Get target angle
  Move((targetPosition - gameObject.GetPosition()).Normalized(), false);
}
