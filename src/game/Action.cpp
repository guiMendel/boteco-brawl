#include "Action.h"
#include "GameObject.h"
#include "Animator.h"
#include "Character.h"

using namespace std;

void AnimationAction::Trigger(GameObject &target, shared_ptr<CharacterState> actionState)
{
  // Store these info
  auto character = target.RequireComponent<Character>();
  int stateId = actionState->id;

  // Start this animation
  // When animation is over, make sure this action's state is no longer active
  target.RequireComponent<Animator>()->Play(GetAnimation(), [stateId, character]()
                                            { if (character) character->RemoveState(stateId); });
}

int AttackAction::GetPriority() const { return 1; }

shared_ptr<CharacterState> AttackAction::NextState(shared_ptr<Action> sharedAction) { return CharacterStateRecipes::Attacking(sharedAction); }
