#ifndef __CHARACTER_CONTROLLER__
#define __CHARACTER_CONTROLLER__

#include "GameObject.h"
#include "Component.h"
#include "Movement.h"
#include "CharacterStateManager.h"
#include "Action.h"
#include <unordered_set>

// Maps input from some source (player or AI) to the the creation and dispatch of the corresponding actions
class CharacterController : public Component
{
public:
  CharacterController(GameObject &associatedObject);
  virtual ~CharacterController() {}

private:
  void Start() override;
  void Update(float deltaTime) override;

  template <class ActionType, typename... Args>
  void Dispatch(Args &&...args)
  {
    character.Perform(std::make_shared<ActionType>(std::forward<Args>(args)...), true);
  }

  template <class ActionType, typename... Args>
  void DispatchNonDelayable(Args &&...args)
  {
    character.Perform(std::make_shared<ActionType>(std::forward<Args>(args)...), false);
  }

  void HandleMovementAnimation();

  void DispatchDash(Vector2 direction);

  void OnLand();

  // Whether an air dash is available
  bool airDashAvailable{true};

  // Current seconds remaining before a dash can be used again
  float dashCooldown{0};

  // How long (seconds) character must wait between successive dashes
  static const float totalDashCooldown;

  CharacterStateManager &character;
  Movement &movement;
  Rigidbody &rigidbody;
  Animator &animator;
};

#endif