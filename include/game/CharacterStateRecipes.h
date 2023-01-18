#ifndef __CHARACTER_STATE_RECIPES__
#define __CHARACTER_STATE_RECIPES__

#include <string>
#include <memory>

#define MOVING_STATE "moving"
#define DASHING_STATE "dashing"
#define JUMPING_STATE "jumping"
#define ATTACKING_STATE "attacking"
#define AIR_ATTACKING_STATE "airAttacking"
#define SPECIAL_ATTACKING_STATE "specialAttacking"
#define LANDING_STATE "landing"
#define RECOVERING_STATE "recovering"

class CharacterState;
class Action;

class CharacterStateRecipes
{
public:
  static std::shared_ptr<CharacterState> Moving(std::shared_ptr<Action>);
  static std::shared_ptr<CharacterState> Dashing(std::shared_ptr<Action>);
  static std::shared_ptr<CharacterState> Jumping(std::shared_ptr<Action>);
  static std::shared_ptr<CharacterState> Attacking(std::shared_ptr<Action>);
  static std::shared_ptr<CharacterState> AirAttacking(std::shared_ptr<Action>);
  static std::shared_ptr<CharacterState> SpecialAttacking(std::shared_ptr<Action>);
  static std::shared_ptr<CharacterState> Landing(std::shared_ptr<Action>);
  static std::shared_ptr<CharacterState> Recovering(std::shared_ptr<Action>);
};

#endif