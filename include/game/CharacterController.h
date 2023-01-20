#ifndef __CHARACTER_CONTROLLER__
#define __CHARACTER_CONTROLLER__

#include "GameObject.h"
#include "Component.h"
#include "Movement.h"
#include "CharacterStateManager.h"
#include "Action.h"
#include "Damage.h"
#include <unordered_set>

// Maps input from some source (player or AI) to the the creation and dispatch of the corresponding actions
class CharacterController : public Component
{
public:
  CharacterController(GameObject &associatedObject);
  virtual ~CharacterController() {}

  // Dispatches either a take damage action or a riposte action
  void TakeHit(Damage damage);

private:
  void Start() override;
  void Update(float deltaTime) override;

  template <class ActionType, typename... Args>
  void Dispatch(Args &&...args)
  {
    stateManager.Perform(std::make_shared<ActionType>(std::forward<Args>(args)...), true);
  }

  template <class ActionType, typename... Args>
  void DispatchNonDelayable(Args &&...args)
  {
    stateManager.Perform(std::make_shared<ActionType>(std::forward<Args>(args)...), false);
  }

  void HandleMovementAnimation();

  void DispatchDash(Vector2 direction);

  void DispatchRiposte(DamageParameters damageParams);

  void OnLand();

  // Warns all states with this name that the action input was released
  // Also warn the queued action if it will produce this state
  void AnnounceInputRelease(std::string targetState);

  // Whether an air dash is available
  bool airDashAvailable{true};

  // Current seconds remaining before a dash can be used again
  float dashCooldown{0};

  // How long (seconds) character must wait between successive dashes
  static const float totalDashCooldown;

  CharacterStateManager &stateManager;
  Movement &movement;
  Rigidbody &rigidbody;
  Animator &animator;
};

#endif