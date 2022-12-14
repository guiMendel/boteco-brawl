#include "CharacterStateRecipes.h"
#include "CharacterState.h"
#include "Action.h"

using namespace std;

shared_ptr<CharacterState> CharacterStateRecipes::Moving(shared_ptr<Action> action)
{
  return make_shared<CharacterState>(MOVING_STATE, 1, action);
}

shared_ptr<CharacterState> CharacterStateRecipes::Dashing(shared_ptr<Action> action)
{
  return make_shared<CharacterState>(DASHING_STATE, 3, action);
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
