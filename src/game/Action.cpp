#include "Action.h"
#include "GameObject.h"
#include "Animator.h"
#include "Character.h"

using namespace std;

Action::Action(Callback callback, int priority, state_getter getState, unordered_set<string> friendStates)
    : callback(callback), priority(priority), getState(getState), friendStates(friendStates) {}

AnimationAction::AnimationAction(string animation, int priority, state_getter getState, unordered_set<string> friendStates)
    : Action([animation, this](GameObject &gameObject, shared_ptr<CharacterState> state)
             { Trigger(animation, gameObject, state); },
             priority, getState, friendStates) {}

void AnimationAction::Trigger(string animation, GameObject &gameObject, shared_ptr<CharacterState> state)
{
  // Start this animation
  // When animation is over, make sure this action's state is no longer active
  gameObject.RequireComponent<Animator>()->Play(animation, [state, gameObject]()
                                                { gameObject.RequireComponent<Character>()->RemoveState(state->id); });
}

AttackAction::AttackAction(string animation, unordered_set<string> friendStates)
    : AnimationAction(animation, 2, CharacterStateRecipes::Attacking, friendStates) {}
