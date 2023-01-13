#ifndef __STATEFUL_ANIMATION__
#define __STATEFUL_ANIMATION__

#include "Animation.h"
#include "CharacterState.h"

// An animation that was created by an action and has a reference to the action's resulting state
class StatefulAnimation : public Animation
{
public:
  StatefulAnimation(std::shared_ptr<Animator> animator);
  virtual ~StatefulAnimation() {}

  // Register the state
  void RegisterState(std::shared_ptr<CharacterState> actionState);

protected:
  // At which frame this action's state is removed
  // A negative value means only when animation stops (default implementation)
  virtual int CancelFrame() const;

  // At which frame this action's state becomes open to sequences
  // A negative value means never (default implementation)
  virtual int OpenSequenceFrame() const;

private:
  std::weak_ptr<CharacterState> weakActionState;
};

#endif
