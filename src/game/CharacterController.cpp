#include "CharacterController.h"
#include "CharacterStateRecipes.h"
#include "Action.h"
#include "PlayerInput.h"

// Action priorities
#define MOVEMENT_PRIORITY 1
#define JUMP_PRIORITY 1
#define LAND_PRIORITY 2

using namespace std;

CharacterController::CharacterController(GameObject &associatedObject)
    : Component(associatedObject),
      character(*gameObject.RequireComponent<Character>()),
      movement(*gameObject.RequireComponent<Movement>()),
      rigidbody(*gameObject.RequireComponent<Rigidbody>()),
      animator(*gameObject.RequireComponent<Animator>()) {}

void CharacterController::Update([[maybe_unused]] float deltaTime)
{
  HandleMovementAnimation();
}

void CharacterController::HandleMovementAnimation()
{
  // Moving animations
  auto states = character.GetStates();

  // Whether just moving
  bool justMoving = states.size() == 1 && states.front()->name == "moving";

  if (movement.IsGrounded())
  {
    // When grounded, run animation if moving is the only state
    if (justMoving)
    {
      // See if moving in the same direction as input
      if (GetSign(movement.GetDirection()) == GetSign(rigidbody.velocity.x))
        animator.Play("run");

      // If not, use brake animation
      else
        animator.Play("brake");
    }

    // Otherwise, stop run animation if it's playing
    else if (animator.GetCurrentAnimation() == "run")
      animator.Play("idle");
  }

  else
    // When not grounded, rise & fall animations
    if (justMoving || states.size() == 0)
      animator.Play(rigidbody.velocity.y >= 0 ? "fall" : "rise");
}

void CharacterController::Start()
{
  // For now, there must be player input. In the future there may be an AIInput instead
  auto input = gameObject.RequireComponent<PlayerInput>();
  auto animator = gameObject.RequireComponent<Animator>();

  // Subscribe to movement
  input->OnMoveDirection.AddListener("character-controller", [this](float direction)
                                     { DispatchMovement(direction); });

  // Subscribe to jumps
  // Make it a friend of moving
  input->OnJump.AddListener("character-controller", [this, animator]()
                            { if (movement.CanJump() ) DispatchAnimation("jump", JUMP_PRIORITY, CharacterStateRecipes::Jumping, unordered_set({"moving"s})); });

  // Fast falling isn't an action
  input->OnFastFall.AddListener("character-controller", [this]()
                                { if (character.HasControl()) movement.FallFast(); });
  input->OnFastFallStop.AddListener("character-controller", [this]()
                                    { movement.StopFallFast(); });

  // Land behavior
  movement.OnLand.AddListener("character-controller", [this]()
                              { DispatchAnimation("land", LAND_PRIORITY, CharacterStateRecipes::Landing, unordered_set({"moving"s})); });
}

void CharacterController::Dispatch(Action::Callback callback, int priority, Action::state_getter getState, unordered_set<string> friends)
{
  character.Perform(make_shared<Action>(callback, priority, getState, friends));
}

void CharacterController::DispatchAnimation(string animation, int priority, Action::state_getter getState, unordered_set<string> friends)
{
  character.Perform(make_shared<AnimationAction>(animation, priority, getState, friends));
}

void CharacterController::DispatchAttack(string animation, unordered_set<string> friends)
{
  character.Perform(make_shared<AttackAction>(animation, friends));
}

void CharacterController::DispatchMovement(float direction)
{
  // Create action
  // If direction is 0, do not push a moving state
  auto moveAction = make_shared<Action>([this, direction](GameObject &, [[maybe_unused]] auto _)
                                        { movement.SetDirection(direction); },
                                        MOVEMENT_PRIORITY,
                                        direction == 0 ? nullptr : CharacterStateRecipes::Moving,
                                        // Add jump as friend state
                                        unordered_set({"jumping"s, "landing"s}));

  // Add a stop callback to stop movement
  moveAction->stopCallback = [this](GameObject &)
  {
    movement.SetDirection(0);
  };

  character.Perform(moveAction);
}