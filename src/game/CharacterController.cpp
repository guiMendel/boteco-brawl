#include "CharacterController.h"
#include "CharacterStateRecipes.h"
#include "Action.h"
#include "Actions.h"
#include "PlayerInput.h"

using namespace std;

CharacterController::CharacterController(GameObject &associatedObject)
    : Component(associatedObject),
      character(*gameObject.RequireComponent<Character>()),
      movement(*gameObject.RequireComponent<Movement>()),
      rigidbody(*gameObject.RequireComponent<Rigidbody>()),
      animator(*gameObject.RequireComponent<Animator>()) {}

void CharacterController::Update(float)
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
                                     { Dispatch<Actions::Move>(direction); });

  // Subscribe to jumps
  // Make it a friend of moving
  input->OnJump.AddListener("character-controller", [this, animator]()
                            { if (movement.CanJump() ) Dispatch<Actions::Jump>(); });

  // Fast falling isn't an action
  input->OnFastFall.AddListener("character-controller", [this]()
                                { if (character.HasControl()) movement.FallFast(); });
  input->OnFastFallStop.AddListener("character-controller", [this]()
                                    { movement.StopFallFast(); });

  // Land behavior
  movement.OnLand.AddListener("character-controller", [this]()
                              { Dispatch<Actions::Land>(); });
}
