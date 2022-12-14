#include "Actions.h"
#include "Movement.h"

using namespace std;
using namespace Actions;

// ============================= MOVE =============================

void Move::Trigger(GameObject &target, shared_ptr<CharacterState>)
{
  target.RequireComponent<Movement>()->SetDirection(direction);
}
void Move::StopHook(GameObject &target)
{
  target.RequireComponent<Movement>()->SetDirection(0);
}
