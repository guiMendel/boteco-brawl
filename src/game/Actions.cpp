#include "Actions.h"
#include "Movement.h"
#include "Rigidbody.h"
#include "Character.h"
#include "ParticleEmitter.h"
#include "ObjectRecipes.h"

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

// ============================= DASH =============================

const float Dash::dashFriction{0.995};
const float Dash::dashSpeed{50};

void Dash::Trigger(GameObject &target, shared_ptr<CharacterState> dashState)
{
  // Get the rigidbody
  auto rigidbody = target.GetComponent<Rigidbody>();
  auto character = target.GetComponent<Character>();

  // Ignore gravity throughout dash
  rigidbody->gravityScale = Vector2(0, 0.01);

  // Use dash friction
  rigidbody->airFriction = dashFriction;

  // Apply dash velocity
  rigidbody->velocity = direction * dashSpeed;

  // Align facing direction to dash direction
  if (direction.x != 0)
    target.localScale.x = Helper::GetSign(direction.x);

  // Disable character control
  character->SetControl(false);

  // Store these info
  int dashStateId = dashState->id;

  // Prepare the recovering state
  auto recoveringState = CharacterStateRecipes::Recovering(dashState->parentAction);
  int recoveringStateId = recoveringState->id;

  // Add a callback to the animation to switch to recovering state
  // Animator::frame_callbacks dashCallbacks = {};
  auto weakState{weak_ptr(dashState)};
  auto weakCharacter{weak_ptr(character)};
  auto weakRecoveringState{weak_ptr(recoveringState)};

  Animator::frame_callbacks dashCallbacks = {
      {2, [weakState, weakCharacter, weakRecoveringState, this]()
       {
         IF_LOCK(weakState, dashState)
         IF_LOCK(weakCharacter, character) IF_LOCK(weakRecoveringState, recoveringState)
         {
           dashState->parentAction = nullptr;
           character->SetState(recoveringState, GetFriendStates());
         }
       }}};

  // Start this animation
  // When animation is over, make sure this action's states are disabled
  target.RequireComponent<Animator>()->Play("dash", dashCallbacks, [dashStateId, recoveringStateId, weakCharacter]()
                                            { IF_LOCK(weakCharacter, character) {
                                              character->RemoveState(dashStateId);
                                              character->RemoveState(recoveringStateId);
                                            } });

  // Play particles
  auto particleObject = target.GetChild(DASH_PARTICLES_OBJECT);

  Assert(particleObject != nullptr, target.GetName() + " had no " DASH_PARTICLES_OBJECT " child");

  particleObject->RequireComponent<ParticleEmitter>()->StartEmission();
}

void Dash::StopHook(GameObject &target)
{
  auto rigidbody = target.GetComponent<Rigidbody>();

  // Restore original values
  rigidbody->gravityScale = Vector2::One();
  rigidbody->airFriction = 0;
  target.GetComponent<Character>()->SetControl(true);

  // Stop particles
  target.GetChild(DASH_PARTICLES_OBJECT)->RequireComponent<ParticleEmitter>()->Stop();
}
