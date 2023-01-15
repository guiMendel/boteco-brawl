#include "Action.h"
#include "GameObject.h"
#include "Animator.h"
#include "StatefulAnimation.h"
#include "CharacterStateManager.h"
#include "Character.h"
#include "PlayerInput.h"
#include "Attack.h"

using namespace std;

bool Action::operator==(const Action &other) { return typeid(*this) == typeid(other); }
bool Action::operator!=(const Action &other) { return !(*this == other); }

void AnimationAction::Trigger(GameObject &target, shared_ptr<CharacterState> actionState)
{
  // Store these info
  auto weakStateManager = weak_ptr(target.RequireComponent<CharacterStateManager>());
  auto weakObject = weak_ptr(target.GetShared());
  auto weakActionState = weak_ptr(actionState);
  int stateId = actionState->id;

  // Get animator
  auto animator = target.RequireComponent<Animator>();

  // Get animation name to use
  string animationName{GetAnimationName()};

  // If the bare animation name is present, use it always, ignoring sequence index
  // Otherwise, append transformed sequence index
  if (animator->HasAnimation(animationName) == false)
  {
    // Get character
    auto character = target.RequireComponent<Character>();

    // Append it, but first convert from 0-based to 1-based
    animationName += to_string(character->TransformSequenceIndexFor(animationName, sequenceIndex) + 1);
  }

  // Get the animation and convert it to stateful
  auto animation = dynamic_pointer_cast<StatefulAnimation>(animator->BuildAnimation(animationName));
  Assert(animation != nullptr, "Action generated an animation which wasn't stateful");

  // Register the action's state to the animation
  animation->RegisterState(actionState);

  // Give it a stop callback
  auto removeStateCallback = [stateId, weakStateManager, weakObject]()
  {
    // When animation is over, make sure this action's state is no longer active
    IF_LOCK(weakStateManager, stateManager)
    {
      stateManager->RemoveState(stateId);
    }
  };
  animation->OnStop.AddListener("parent-action-cleanup", removeStateCallback);

  // Start this animation
  animator->Play(animation, true);
}

int AttackAction::GetPriority() const { return ATTACK_PRIORITY; }

shared_ptr<CharacterState> AttackAction::NextState(shared_ptr<Action> sharedAction)
{
  return CharacterStateRecipes::Attacking(sharedAction);
}

void AttackAction::Trigger(GameObject &target, shared_ptr<CharacterState> actionState)
{
  // Ensure character orientation is set to PlayerInput direction
  auto playerInput = target.RequireComponent<PlayerInput>();

  if (abs(playerInput->GetCurrentMoveDirection()) > 0.1)
    target.localScale.x = GetSign(playerInput->GetCurrentMoveDirection());

  // Default behavior
  AnimationAction::Trigger(target, actionState);
}
