#include "Action.h"
#include "GameObject.h"
#include "Animator.h"
#include "CharacterStateManager.h"

using namespace std;

void AnimationAction::Trigger(GameObject &target, shared_ptr<CharacterState> actionState)
{
  // Store these info
  auto weakStateManager = weak_ptr(target.RequireComponent<CharacterStateManager>());
  int stateId = actionState->id;

  // Get animator
  auto animator = target.RequireComponent<Animator>();

  // Get animation name to use
  string animation;

  // If the bare animation name is present, use it always
  if (animator->HasAnimation(GetAnimation()))
    animation = GetAnimation();

  // Otherwise, append transformed sequence index
  else
    animation = GetAnimation() + to_string(TransformIndex());

  // Start this animation
  // When animation is over, make sure this action's state is no longer active
  animator->Play(animation, [stateId, weakStateManager]()
                 { IF_LOCK(weakStateManager, stateManager)
                                              stateManager->RemoveState(stateId); });
}

// Default implementation does no transformation
int AnimationAction::TransformIndex() const { return sequenceIndex; }

int AttackAction::GetPriority() const { return 1; }

shared_ptr<CharacterState> AttackAction::NextState(shared_ptr<Action> sharedAction) { return CharacterStateRecipes::Attacking(sharedAction); }
