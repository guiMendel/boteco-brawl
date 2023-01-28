#ifndef __CHARACTER_STATE_MANAGER__
#define __CHARACTER_STATE_MANAGER__

#include "WorldObject.h"
#include "Component.h"
#include "CharacterState.h"
#include "CharacterStateRecipes.h"
#include "Rigidbody.h"
#include "Event.h"
#include <list>
#include <unordered_set>

// An arena character that have multiple states, such as moving and jumping
class CharacterStateManager : public Component
{
public:
  CharacterStateManager(WorldObject &associatedObject);
  virtual ~CharacterStateManager() {}

  void Awake() override;

  void PhysicsUpdate(float) override;
  void Update(float) override;

  // =================================
  // SELF CONTROL
  // =================================
public:
  // When control is lost or recovered
  EventI<bool> OnControlChange;

  // Minimum speed at which character will bounce off of surfaces (even the ground)
  static const float minBounceSpeed;

  // Whether character has control of self
  bool HasControl() const;

  // Whether character is stunned and going fast enough to bounce on surfaces
  bool IsBouncing() const;

private:
  // Sets character to start bouncing
  void StartBouncing();

  // Stops bouncing
  void StopBouncing();

  // Keeps track of whether character had control last frame
  bool lastFrameControl{true};

  // =================================
  // STATE MANAGEMENT
  // =================================
public:
  // Triggered when a state exits normally (times out)
  EventI<std::shared_ptr<CharacterState>> OnCompleteState;

  // Triggered when an action interrupts the current state
  EventI<std::shared_ptr<CharacterState>> OnInterruptState;

  // When states becomes empty
  Event OnEnterIdle;

  // Get the current states
  const std::list<std::shared_ptr<CharacterState>> GetStates() const;

  // Add a new state to the list, and remove any states not in the keepList
  void SetState(std::shared_ptr<CharacterState> newState, std::unordered_set<std::string> keepStates = {});

  // Whether a given state is present
  bool HasState(std::string stateName);

  // Removes any states not in the list
  void RemoveStatesNotIn(std::unordered_set<std::string> keepStates, bool interruption = false);

  // Removes a state from it's unique id
  void RemoveState(unsigned id, bool interruption = false);

  // Remove a state given it's name if it's present
  void RemoveState(std::string name, bool interruption = false);

  // Get queued action, if any
  std::shared_ptr<Action> GetQueuedAction() const;

  // If the action's priority is high enough, trigger it for this character and set it's state accordingly
  // The canDelay param specifies if this action can wait until the current state is over, if it's not able to interrupt it immediately
  void Perform(std::shared_ptr<Action> action, bool canDelay = false);

private:
  // Current states of the character
  std::list<std::shared_ptr<CharacterState>> states;

  // Performs queued action if possible
  void HandleQueuedAction(float deltaTime);

  // Checks if any states have requested removal
  void HandleStateExpiration();

  // Adds a state, without checking for priorities
  void AddState(std::shared_ptr<CharacterState> newState);

  // Puts an action in the queue
  void QueueAction(std::shared_ptr<Action> action);

  // Deletes anything in the queue
  void ResetQueue();

  // Checks if character is able to perform this action this frame
  bool CanPerform(std::shared_ptr<Action> action);

  // Sets the action's sequence index based on current state
  void SetActionSequenceIndex(std::shared_ptr<Action> action);

  // Clears up and removes state from list
  auto RemoveState(decltype(states)::iterator stateIterator, bool interruption, bool ignoreIdleEvent = false) -> decltype(states)::iterator;

  // Action waiting to be performed as soon as it's able to
  std::shared_ptr<Action> queuedAction;

  // How much time the queued action still has to trigger before being discarded
  float queuedActionTTL{0};

  // Max seconds an action can stay in the queue before being discarded
  static const float maxActionDelay;

  // =================================
  // UTILITY
  // =================================
public:
  std::shared_ptr<CharacterStateManager> GetSharedCasted() const;

private:
  std::weak_ptr<Rigidbody> weakBody;
};

#endif