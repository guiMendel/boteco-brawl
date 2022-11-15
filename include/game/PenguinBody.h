#ifndef __PENGUIN_BODY__
#define __PENGUIN_BODY__

#include "GameObject.h"
#include "Component.h"
#include "Movement.h"
#include <memory>

class PenguinBody : public Component
{
public:
  // From 0 to 1, per second
  static const float proportionAcceleration;
  static const float acceleration;
  static const float maxSpeed;
  static const float totalHealth;

  // In radians per second
  static const float rotationSpeed;

  PenguinBody(GameObject &associatedObject, std::weak_ptr<Movement> movementWeak)
      : Component(associatedObject), movementWeak(movementWeak) {}

  virtual ~PenguinBody() {}

  RenderLayer GetRenderLayer() override { return RenderLayer::None; }

  void Update(float deltaTime) override;
  void OnBeforeDestroy() override;

private:
  void Accelerate(float deltaTime);
  void Rotate(float deltaTime);

  // Current max speed proportion
  float speedProportion{0.0f};

  // Access facilitator
  std::weak_ptr<Movement> movementWeak;
};

#endif