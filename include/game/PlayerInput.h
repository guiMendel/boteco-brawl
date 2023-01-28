#ifndef __PLAYER_CONTROLLER__
#define __PLAYER_CONTROLLER__

#include "Component.h"
#include "Event.h"
#include "Movement.h"

class Movement;

class PlayerInput : public Component
{
public:
  PlayerInput(WorldObject &associatedObject);
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

  Event OnAttackNeutral;
  Event OnAttackHorizontal;
  Event OnAttackUp;

  Event OnAirHorizontal;
  Event OnAirUp;
  Event OnAirDown;

  Event OnReleaseAttack;

  Event OnSpecialNeutral;
  Event OnSpecialHorizontal;

  Event OnReleaseSpecial;

  float GetCurrentMoveDirection() const;

protected:
  // Sets direction to 0, if current direction equals the provided one (both have the same sign)
  void CancelMoveDirection(float direction);

  // When player hits the attack button
  void Attack();

  // When player hits the special button
  void Special();

  // Issues new direction
  void SetDirection(Vector2 direction);

  // Last detected input direction
  Vector2 currentDirection;

  // Movement reference
  std::weak_ptr<Movement> weakMovement;
};

#endif