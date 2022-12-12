#ifndef __CHARACTER_STATE__
#define __CHARACTER_STATE__

#include <string>
#include <memory>

class Action;

struct CharacterState
{
  // Unique identifier pool
  static unsigned idGenerator;

  // State unique identifier
  const unsigned id{idGenerator++};

  // State identifier
  const std::string identifier;

  // Interruption priority of this state
  // Only actions with this priority or higher will be allowed to interrupt this state
  const int priority;

  // The action that resulted in this state (if any)
  const std::shared_ptr<Action> parentAction;

  CharacterState(std::string identifier, int priority, std::shared_ptr<Action> parentAction = nullptr)
      : identifier(identifier), priority(priority), parentAction(parentAction) {}
};

#endif