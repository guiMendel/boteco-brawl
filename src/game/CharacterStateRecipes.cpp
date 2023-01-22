#include "CharacterStateRecipes.h"
#include "CharacterState.h"
#include "CharacterStateManager.h"
#include "Action.h"
#include "Actions.h"

using namespace std;

// Max speed from which character can recover from stunned state
static const float stunRecoverSpeed{2};

shared_ptr<CharacterState> CharacterStateRecipes::Moving(shared_ptr<Action> action)
{
  return make_shared<CharacterState>(MOVING_STATE, 1, action);
}

shared_ptr<CharacterState> CharacterStateRecipes::Dashing(shared_ptr<Action> action)
{
  auto state = make_shared<CharacterState>(DASHING_STATE, 3, action);

  state->losesControl = true;

  return state;
}

shared_ptr<CharacterState> CharacterStateRecipes::Jumping(shared_ptr<Action> action)
{
  return make_shared<CharacterState>(JUMPING_STATE, 2, action);
}

shared_ptr<CharacterState> CharacterStateRecipes::Landing(shared_ptr<Action> action)
{
  return make_shared<CharacterState>(LANDING_STATE, 2, action);
}

shared_ptr<CharacterState> CharacterStateRecipes::Attacking(shared_ptr<Action> action)
{
  return make_shared<CharacterState>(ATTACKING_STATE, 2, action);
}

shared_ptr<CharacterState> CharacterStateRecipes::AirAttacking(shared_ptr<Action> action)
{
  return make_shared<CharacterState>(AIR_ATTACKING_STATE, 2, action);
}

shared_ptr<CharacterState> CharacterStateRecipes::SpecialAttacking(shared_ptr<Action> action)
{
  return make_shared<CharacterState>(SPECIAL_ATTACKING_STATE, 2, action);
}

shared_ptr<CharacterState> CharacterStateRecipes::Recovering(shared_ptr<Action> action)
{
  return make_shared<CharacterState>(RECOVERING_STATE, 2, action);
}

shared_ptr<CharacterState> CharacterStateRecipes::Stunned(shared_ptr<Action> action)
{
  auto state = make_shared<CharacterState>(STUNNED_STATE, 2, action);

  state->losesControl = true;

  state->removeCondition = [](shared_ptr<CharacterStateManager> stateManager)
  {
    // cout << stateManager->gameObject.timer.Get(STUN_DURATION_TIMER) << endl;
    
    // Check if stun time is up
    if (stateManager->gameObject.timer.Get(STUN_DURATION_TIMER) < 0)
      return false;

    // Check if velocity is low enough
    return stateManager->gameObject.RequireComponent<Rigidbody>()->velocity.SqrMagnitude() <=
           stunRecoverSpeed * stunRecoverSpeed;
  };

  return state;
}
