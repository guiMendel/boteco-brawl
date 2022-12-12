#include "CharacterStateRecipes.h"
#include "CharacterState.h"
#include "Action.h"

using namespace std;

shared_ptr<CharacterState> CharacterStateRecipes::Moving(shared_ptr<Action> action)
{
  return make_shared<CharacterState>("moving", 1, action);
}

shared_ptr<CharacterState> CharacterStateRecipes::Dashing(shared_ptr<Action> action)
{
  return make_shared<CharacterState>("dashing", 2, action);
}

shared_ptr<CharacterState> CharacterStateRecipes::Jumping(shared_ptr<Action> action)
{
  return make_shared<CharacterState>("jumping", 2, action);
}

shared_ptr<CharacterState> CharacterStateRecipes::Attacking(shared_ptr<Action> action)
{
  return make_shared<CharacterState>("attacking", 2, action);
}
