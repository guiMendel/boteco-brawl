#ifndef __CHARACTER_CONTROLLER__
#define __CHARACTER_CONTROLLER__

#include "GameObject.h"
#include "Component.h"
#include "Movement.h"
#include "Character.h"
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

  void Dispatch(Action::Callback callback, int priority, Action::state_getter getState, std::unordered_set<std::string> friends = {});
  void DispatchAnimation(std::string animation, int priority, Action::state_getter getState, std::unordered_set<std::string> friends = {});
  void DispatchAttack(std::string animation, std::unordered_set<std::string> friends = {});

  void DispatchMovement(float direction);

  void HandleMovementAnimation();

  Character &character;
  Movement &movement;
  Rigidbody &rigidbody;
  Animator &animator;
};

#endif