#ifndef __MOVEMENT__
#define __MOVEMENT__

#include <functional>
#include "GameObject.h"
#include "Component.h"
#include "Vector2.h"

class Movement : public Component
{
public:
  Movement(GameObject &associatedObject, float acceleration = 0.0f, float targetSpeed = 0.0f)
      : Component(associatedObject), acceleration(acceleration), targetSpeed(targetSpeed) {}

  virtual ~Movement() {}

  void Update(float deltaTime) override;
  RenderLayer GetRenderLayer() override { return RenderLayer::None; }

  // Start moving towards this target
  void MoveTo(Vector2 position, std::function<void()> callback = nullptr);

  Vector2 GetDirection() { return targetDirection; }

  void Move(Vector2 direction, bool cancelFollow = true);

private:
  // Update speed to match target direction
  void Accelerate(float deltaTime);

  // Current speed
  Vector2 velocity{0, 0};

  // Target direction of current movement
  Vector2 targetDirection;

  // A target to move towards each frame (if followTarget is true)
  Vector2 targetPosition;

  // Whether to follow current target
  bool followTarget{false};

  // Acceleration, scaled from 0 to 1, in targetSpeed per second
  float acceleration;

  // Speed to accelerate to when direction magnitude is 1
  float targetSpeed;

  // Callback to execute on target reach
  std::function<void()> targetReachCallback;

  void FollowTarget(float deltaTime);
};

#endif