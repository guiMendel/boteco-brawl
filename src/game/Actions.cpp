#include "Actions.h"
#include "Movement.h"
#include "Animation.h"
#include "Rigidbody.h"
#include "CharacterStateManager.h"
#include "ParticleEmitter.h"
#include "ObjectRecipes.h"
#include "GeneralAnimations.h"
#include "Heat.h"

using namespace std;
using namespace Actions;

// ============================= MOVE =============================

void Move::Trigger(GameObject &target, shared_ptr<CharacterState>)
{
  target.RequireComponent<Movement>()->SetDirection(direction);
}
void Move::StopHook(GameObject &target, shared_ptr<CharacterState>)
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
  auto character = target.GetComponent<CharacterStateManager>();

  // Ignore gravity throughout dash
  rigidbody->gravityScale = Vector2(0, 0.01);

  // Use dash friction
  rigidbody->airFriction = dashFriction;

  // Apply dash velocity
  rigidbody->velocity = direction * dashSpeed;

  // Align facing direction to dash direction
  target.localScale.x = Helper::GetSign(direction.x, target.localScale.x);

  // Store these info
  int dashStateId = dashState->id;

  // Prepare the recovering state
  auto recoveringState = CharacterStateRecipes::Recovering(dashState->parentAction);
  int recoveringStateId = recoveringState->id;

  // Add a callback to the animation to switch to recovering state
  // Animator::frame_callbacks dashCallbacks = {};
  auto weakState{weak_ptr(dashState)};
  auto weakStateManager{weak_ptr(character)};
  auto weakRecoveringState{weak_ptr(recoveringState)};

  // Get animator
  auto animator = target.RequireComponent<Animator>();

  // Build animation
  auto animation = animator->BuildAnimation("dash");

  // Switch from invulnerable state to recovering state mid-animation
  auto stateSwitchCallback = [weakState, weakStateManager, weakRecoveringState, this](GameObject &)
  {
    IF_LOCK(weakState, dashState)
    IF_LOCK(weakStateManager, stateManager)
    IF_LOCK(weakRecoveringState, recoveringState)
    {
      dashState->parentAction = nullptr;
      stateManager->SetState(recoveringState, GetFriendStates());
    }
  };
  animation->GetFrame(2).AddCallback(stateSwitchCallback);

  // On stop, ensure both states have been removed
  auto stopCallback = [dashStateId, recoveringStateId, weakStateManager]()
  {
    IF_LOCK(weakStateManager, stateManager)
    {
      stateManager->RemoveState(dashStateId);
      stateManager->RemoveState(recoveringStateId);
    }
  };
  animation->OnStop.AddListener("action-cleanup", stopCallback);

  // Start this animation
  animator->Play(animation);

  // Play particles
  auto particleObject = target.GetChild(DASH_PARTICLES_OBJECT);

  Assert(particleObject != nullptr, target.GetName() + " had no " DASH_PARTICLES_OBJECT " child");

  particleObject->RequireComponent<ParticleEmitter>()->StartEmission();
}

void Dash::StopHook(GameObject &target, shared_ptr<CharacterState>)
{
  auto rigidbody = target.GetComponent<Rigidbody>();

  // Restore original values
  rigidbody->gravityScale = Vector2::One();
  rigidbody->airFriction = 0;

  // Stop particles
  target.GetChild(DASH_PARTICLES_OBJECT)->RequireComponent<ParticleEmitter>()->Stop();
}

// ============================= TAKE DAMAGE =============================

void TakeDamage::Trigger(GameObject &target, shared_ptr<CharacterState>)
{
  // Get target's heat
  auto heat = target.RequireComponent<Heat>();

  // Apply the damage to it
  heat->TakeDamage(damage);

  // Start the remove timer
  target.timer.Reset(STUN_DURATION_TIMER, -damage.stunTime);

  // If it has stun time
  if (damage.stunTime > 0)
  {
    // Pick an ouch animation
    string ouchAnimation = "ouch" + to_string(RandomRange(1, 3));

    // Play it
    target.RequireComponent<Animator>()->Play(ouchAnimation);
  }
}

// ============================= RIPOSTE =============================

void Riposte::Trigger(GameObject &target, shared_ptr<CharacterState> actionState)
{
  LOCK(weakParry, parry);

  // Store these info
  auto weakStateManager = weak_ptr(target.RequireComponent<CharacterStateManager>());
  auto weakObject = weak_ptr(target.GetShared());
  auto weakActionState = weak_ptr(actionState);
  int stateId = actionState->id;

  // Get animator
  auto animator = target.RequireComponent<Animator>();

  // Get riposte animation
  auto animation = dynamic_pointer_cast<GeneralAnimations::Riposte>(animator->BuildAnimation("riposte"));
  Assert(animation != nullptr, "Animator built a riposte animation which isn't of the Riposte animation type");

  animation->damage = parry->Riposte(parriedDamage);

  // Give it a stop callback
  auto removeStateCallback = [stateId, weakStateManager]()
  {
    // When animation is over, make sure this action's state is no longer active
    IF_LOCK(weakStateManager, stateManager)
    {
      stateManager->RemoveState(stateId);
    }
  };
  animation->OnStop.AddListener("parent-action-cleanup", removeStateCallback);

  // Start this animation
  animator->Play(animation, true);

  // Ensure character is facing direction of attacker
  LOCK(parriedDamage.weakAuthor, author);

  auto direction = GetSign(author->GetPosition().x - target.GetPosition().x);

  target.localScale.x = direction;
}

// ============================= LANDING ATTACK =============================

void LandingAttack::Trigger(GameObject &target, shared_ptr<CharacterState> actionState)
{
  // Store these info
  auto weakStateManager = weak_ptr(target.RequireComponent<CharacterStateManager>());
  int stateId = actionState->id;

  // Get animator
  auto animator = target.RequireComponent<Animator>();

  // Get landing attack animation
  auto animation = dynamic_pointer_cast<GeneralAnimations::LandingAttack>(animator->BuildAnimation("landingAttack"));
  Assert(animation != nullptr, "Animator built a landing attack animation which isn't of the LandingAttack animation type");

  animation->landingSpeed = landingSpeed;

  // Give it a stop callback
  auto removeStateCallback = [stateId, weakStateManager]()
  {
    // When animation is over, make sure this action's state is no longer active
    IF_LOCK(weakStateManager, stateManager)
    {
      stateManager->RemoveState(stateId);
    }
  };
  animation->OnStop.AddListener("parent-action-cleanup", removeStateCallback);

  // Start this animation
  animator->Play(animation, true);
}
