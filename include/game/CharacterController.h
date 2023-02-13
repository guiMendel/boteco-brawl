#ifndef __CHARACTER_CONTROLLER__
#define __CHARACTER_CONTROLLER__

#include "WorldObject.h"
#include "WorldComponent.h"
#include "Movement.h"
#include "Player.h"
#include "CharacterStateManager.h"
#include "Action.h"
#include "Damage.h"
#include "Animator.h"
#include "Invulnerability.h"
#include <unordered_set>

// Maps input from some source (player or AI) to the the creation and dispatch of the corresponding actions
class CharacterController : public WorldComponent
{
public:
  CharacterController(GameObject &associatedObject, std::shared_ptr<Player> player);
  virtual ~CharacterController() {}

  // Get player associated to this character
  std::shared_ptr<Player> GetPlayer() const;

  // Dispatches either a take damage action or a riposte action
  void TakeHit(Damage damage, bool parryable = true);

private:
  void Start() override;
  void Update(float deltaTime) override;
  void PhysicsUpdate(float deltaTime) override;

  template <class ActionType, typename... Args>
  void Dispatch(bool delayable, bool requiresControl, Args &&...args)
  {
    if (requiresControl == false || controlDisabled == false)
      stateManager.Perform(std::make_shared<ActionType>(std::forward<Args>(args)...), delayable);
  }

  void HandleMovementAnimation();

  void DispatchDash(Vector2 direction);

  void OnLand();

  // Warns all states with this name that the action input was released
  // Also warn the queued action if it will produce this state
  void AnnounceInputRelease(std::string targetState);

  // Whether character control is disabled
  bool controlDisabled{true};

  // Whether an air dash is available
  bool airDashAvailable{true};

  // Current seconds remaining before a dash can be used again
  float dashCooldown{0};

  // How long (seconds) character must wait between successive dashes
  static const float totalDashCooldown;

  // Player associated to this character
  std::weak_ptr<Player> weakPlayer;

  Vector2 lastVelocity;

  Invulnerability &invulnerability;
  CharacterStateManager &stateManager;
  Movement &movement;
  Rigidbody &rigidbody;
  Animator &animator;
};

#endif