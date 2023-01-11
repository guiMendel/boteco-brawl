#include "Action.h"
#include "GameObject.h"
#include "Animator.h"
#include "CharacterStateManager.h"
#include "Character.h"

using namespace std;

void AnimationAction::Trigger(GameObject &target, shared_ptr<CharacterState> actionState)
{
  // Store these info
  auto weakStateManager = weak_ptr(target.RequireComponent<CharacterStateManager>());
  int stateId = actionState->id;

  // Get animator
  auto animator = target.RequireComponent<Animator>();

  // Get animation name to use
  string animation{GetAnimation()};

  // If the bare animation name is present, use it always, ignoring sequence index
  // Otherwise, append transformed sequence index
  if (animator->HasAnimation(animation) == false)
  {
    // Get character
    auto character = target.RequireComponent<Character>();

    // Append it, but first convert from 0-based to 1-based
    animation += to_string(character->TransformSequenceIndexFor(animation, sequenceIndex) + 1);
  }

  // Start this animation
  // When animation is over, make sure this action's state is no longer active
  animator->Play(animation, [stateId, weakStateManager]()
                 { IF_LOCK(weakStateManager, stateManager)
                                              stateManager->RemoveState(stateId); });
}

int AttackAction::GetPriority() const { return 1; }

shared_ptr<CharacterState> AttackAction::NextState(shared_ptr<Action> sharedAction) { return CharacterStateRecipes::Attacking(sharedAction); }
