#ifndef __CHARACTER_STATE_RECIPES__
#define __CHARACTER_STATE_RECIPES__

#include <string>
#include <memory>

class CharacterState;
class Action;

class CharacterStateRecipes
{
public:
  static std::shared_ptr<CharacterState> Moving(std::shared_ptr<Action>);
  static std::shared_ptr<CharacterState> Dashing(std::shared_ptr<Action>);
  static std::shared_ptr<CharacterState> Jumping(std::shared_ptr<Action>);
  static std::shared_ptr<CharacterState> Attacking(std::shared_ptr<Action>);
};

#endif