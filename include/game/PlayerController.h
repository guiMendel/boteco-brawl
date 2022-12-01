#ifndef __PLAYER_CONTROLLER__
#define __PLAYER_CONTROLLER__

#include "GameObject.h"
#include "Component.h"

class Movement;

class PlayerController : public Component
{
public:
  PlayerController(GameObject &associatedObject);
  virtual ~PlayerController() {}

  void Update(float deltaTime) override;

private:
  // Sets direction to 0, if current direction equals the provided one (both have the same sign)
  void CancelDirection(float direction);

  // Issues new direction
  void SetDirection(float direction);

  // Reference to movement component
  Movement &movement;

  // Last direction issued to Movement
  float currentDirection{0};
};

#endif