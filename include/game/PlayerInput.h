#ifndef __PLAYER_CONTROLLER__
#define __PLAYER_CONTROLLER__

#include "GameObject.h"
#include "Component.h"
#include "Event.h"

class Movement;

class PlayerInput : public Component
{
public:
  PlayerInput(GameObject &associatedObject);
  virtual ~PlayerInput() {}

  // Raised on movement input
  EventI<float> OnMoveDirection;

  // Raised on dash
  EventI<Vector2> OnDash;

  // Raised on jump input
  Event OnJump;

  // Raised on fast fall
  Event OnFastFall;

  // Raised on stop fast fall
  Event OnFastFallStop;

  Event OnNeutralAttack;
  
  Event OnNeutralSpecial;

  float GetCurrentMoveDirection() const;

protected:
  // Sets direction to 0, if current direction equals the provided one (both have the same sign)
  void CancelDirection(float direction);

  // Issues new direction
  void SetDirection(float direction);

private:
  // Last direction issued to Movement
  float currentDirection{0};
};

#endif