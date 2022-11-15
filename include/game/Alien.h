#ifndef __ALIEN__
#define __ALIEN__

#include "GameObject.h"
#include "Movement.h"
#include "PenguinBody.h"
#include "Health.h"
#include "Vector2.h"
#include "Component.h"
#include <queue>
#include <vector>
#include <memory>

class Alien : public Component
{
public:
  // Rotation speed, in radians
  static const float rotationSpeed;

  // Movement acceleration
  static const float acceleration;
  static const float maxSpeed;

  // Min and max total minion count
  static const Vector2 totalMinions;

  // Total health points
  static const float healthPoints;

  // Min and max time to stay idle
  static const Vector2 idleTime;

  Alien(GameObject &associatedObject);
  virtual ~Alien() {}

  void OnBeforeDestroy() override;
  void Start() override;
  void Update(float deltaTime) override;

  // It's current minions
  std::vector<std::weak_ptr<GameObject>> minions;

private:
  void Chase();
  void Arrive();
  void Shoot(Vector2 position);

  enum class State
  {
    moving,
    idle
  };

  State state{State::idle};

  std::weak_ptr<Movement> movementWeak;
  std::weak_ptr<PenguinBody> penguinWeak;
};

#endif