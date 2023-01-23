#ifndef __CHARACTER_STATE__
#define __CHARACTER_STATE__

#include <string>
#include <memory>
#include "Event.h"

class Action;
class CharacterStateManager;

class CharacterState
{
public:
  // Raises when button that triggered parent action is released
  // Only raises for specific states: those originating from attacks & specials
  Event OnActionInputRelease;

  CharacterState(std::string name, int priority, std::shared_ptr<Action> parentAction = nullptr);

  // When this condition is met, the state should be removed
  bool RemoveRequested(std::shared_ptr<CharacterStateManager>);

  // Register action input as released
  void ReleaseActionInput();

  // Whether the button that triggered parent action was already released
  bool ActionInputReleased() const;

  bool operator==(const CharacterState &other) { return name == other.name; }
  bool operator!=(const CharacterState &other) { return !(*this == other); }

  // Unique id pool
  static unsigned idGenerator;

  // State unique identifier
  const unsigned id{idGenerator++};

  // State name
  const std::string name;

  // Interruption priority of this state
  // Only actions with this priority or higher will be allowed to interrupt this state
  const int priority;

  // The action that resulted in this state (if any)
  std::shared_ptr<Action> parentAction;

  // State removal condition callback
  std::function<bool(std::shared_ptr<CharacterStateManager>)> removeCondition{nullptr};

  // Callback to trigger on state add
  std::function<void(std::shared_ptr<CharacterStateManager>)> onAdd{nullptr};

  // Callback to trigger on state remove
  std::function<void(std::shared_ptr<CharacterStateManager>)> onRemove{nullptr};

  // Whether this state can be interrupted by an action of the same type of parent regardless of priority
  bool openToSequence{false};

  // Whether the character will be tagged as out of control as long as it has this state
  bool losesControl{false};

private:
  // Whether the button that triggered parent action was already released
  bool actionInputReleased{false};
};

#endif