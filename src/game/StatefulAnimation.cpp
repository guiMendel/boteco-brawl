#include "StatefulAnimation.h"
#include "GameObject.h"
#include "CharacterStateManager.h"

using namespace std;
using namespace Helper;

StatefulAnimation::StatefulAnimation(std::shared_ptr<Animator> animator) : Animation(animator) {}

void StatefulAnimation::RegisterState(std::shared_ptr<CharacterState> actionState)
{
  // Ensure no state has been previously registered
  Assert(weakActionState.expired(), "Tried to register another state to the same stateful animation");

  // Register it
  weakActionState = actionState;

  // Add callbacks for this state

  // If there's a cancel frame
  if (CancelFrame() >= 0)
  {
    auto stopCallback = [this](GameObject &)
    {
      IF_LOCK(weakActionState, actionState)
      {
        LOCK(weakAnimator, animator)
        animator->gameObject.RequireComponent<CharacterStateManager>()->RemoveState(actionState->id);
      }
    };

    GetFrame(CancelFrame()).AddCallback(stopCallback);
  }

  // Also add open sequence frame
  if (OpenSequenceFrame() >= 0)
  {
    GetFrame(OpenSequenceFrame()).AddCallback([this](GameObject &)
                                              { IF_LOCK(weakActionState, actionState)
                                                           actionState->openToSequence = true; });
  }
}

int StatefulAnimation::CancelFrame() const { return -1; }

int StatefulAnimation::OpenSequenceFrame() const { return -1; }
