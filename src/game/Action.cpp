#include "Action.h"
#include "GameObject.h"
#include "Animator.h"
#include "Animation.h"
#include "CharacterStateManager.h"
#include "Character.h"
#include "Attack.h"

using namespace std;

bool Action::operator==(const Action &other) { return typeid(*this) == typeid(other); }
bool Action::operator!=(const Action &other) { return !(*this == other); }

int AnimationAction::CancelFrame() const { return -1; }

int AnimationAction::OpenSequenceFrame() const { return -1; }

void AnimationAction::Trigger(GameObject &target, shared_ptr<CharacterState> actionState)
{
  // Store these info
  auto weakStateManager = weak_ptr(target.RequireComponent<CharacterStateManager>());
  auto weakObject = weak_ptr(target.GetShared());
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

  // Get the animation
  auto animation = animator->BuildAnimation(animationName);

  // Give it a stop callback
  auto stopCallback = [stateId, weakStateManager, weakObject]()
  {
    // When animation is over, make sure this action's state is no longer active
    IF_LOCK(weakStateManager, stateManager)
    {
      stateManager->RemoveState(stateId);
    }
  };

  animation->OnStop.AddListener("parent-action-cleanup", stopCallback);

  // Start this animation
  animator->Play(animation);
}

int AttackAction::GetPriority() const { return 1; }

shared_ptr<CharacterState> AttackAction::NextState(shared_ptr<Action> sharedAction)
{
  return CharacterStateRecipes::Attacking(sharedAction);
}
