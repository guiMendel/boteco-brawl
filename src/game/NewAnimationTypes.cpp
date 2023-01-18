#include "NewAnimationTypes.h"
#include "GameObject.h"
#include "CharacterStateManager.h"
#include "Attack.h"
#include "CircleCollider.h"

using namespace std;
using namespace Helper;

// === STATEFUL ANIMATIONS

StatefulAnimation::StatefulAnimation(shared_ptr<Animator> animator) : Animation(animator) {}

void StatefulAnimation::RegisterState(shared_ptr<CharacterState> actionState)
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

  // Subscribe to input release events
  if (actionState != nullptr)
    actionState->OnActionInputRelease.AddListener("stateful-animation-" + to_string(id), [this]()
                                                  { OnParentActionInputRelease(); });
}

StatefulAnimation::~StatefulAnimation()
{
  IF_LOCK(weakActionState, actionState)
  {
    // Unsubscribe to input release events
    actionState->OnActionInputRelease.RemoveListener("stateful-animation-" + to_string(id));
  }
}

int StatefulAnimation::CancelFrame() const { return -1; }

int StatefulAnimation::OpenSequenceFrame() const { return -1; }

void StatefulAnimation::OnParentActionInputRelease() {}

shared_ptr<Animation> StatefulAnimation::GetNext()
{
  auto next = GetNextStateful();

  // Pass over state to it
  if (next != nullptr)
    next->RegisterState(weakActionState.lock());

  return next;
}

shared_ptr<StatefulAnimation> StatefulAnimation::GetNextStateful() { return nullptr; }

// === ATTACK ANIMATIONS

DamageParameters AttackAnimation::GetAttackProperties() const
{
  return {0, Vector2{0, 0}, 0};
}

void AttackAnimation::InternalOnStart() { SetupAttack(); }
void AttackAnimation::InternalOnStop()
{
  LOCK(weakAnimator, animator);

  if (attackObjectId >= 0)
    animator->gameObject.RequireChild(attackObjectId)->RequestDestroy();
}

void AttackAnimation::SetupAttack()
{
  LOCK(weakAnimator, animator);

  // Create child
  auto attackObject = animator->gameObject.CreateChild(ATTACK_OBJECT);
  attackObject->SetPhysicsLayer(PhysicsLayer::Hitbox);

  attackObjectId = attackObject->id;

  auto damageParams = GetAttackProperties();

  // Give it the attack component
  attackObject->AddComponent<Attack>(damageParams);
}

void AttackAnimation::FrameHitbox(AnimationFrame &frame, vector<Circle> hitboxAreas)
{
  auto callback = [this, frame, hitboxAreas](GameObject &)
  {
    if (hitboxAreas.empty())
      RemoveHitbox();
    else
      SetHitbox(frame, hitboxAreas);
  };

  frame.AddCallback(callback);
}

void AttackAnimation::SetHitbox(const AnimationFrame &frame, vector<Circle> hitboxAreas)
{
  LOCK(weakAnimator, animator);

  auto attackObject = animator->gameObject.RequireChild(attackObjectId);

  // First, remove all colliders already there
  RemoveHitbox();

  // Get sprite renderer
  auto spriteRenderer = attackObject->GetParent()->RequireComponent<SpriteRenderer>();

  // Global position of sprite's top-left pixel, in units
  Vector2 topLeftPosition = spriteRenderer->RenderPositionFor(attackObject->GetParent()->GetPosition(), frame.GetSprite());

  // Displacement to apply to attackObject's position to get to this pixel's position
  Vector2 spriteOrigin = topLeftPosition - attackObject->GetPosition();

  // When mirrored, we want to displace with reference to top-right pixel, so sum the sprite's width
  // But also keep this offset positive so as to not double-mirror it on render
  if (GetSign(attackObject->GetScale().x) < 0)
    spriteOrigin.x = -(spriteOrigin.x + frame.GetSprite()->GetWidth());

  // Now, add each provided area as a collider
  for (auto circle : hitboxAreas)
  {
    // Convert circle's virtual pixels to units, and also make it relative to the center of the top left pixel
    circle = Circle(
        spriteOrigin + (circle.center + Vector2{0.5, 0.5}) / float(Game::defaultVirtualPixelsPerUnit),
        circle.radius / float(Game::defaultVirtualPixelsPerUnit));

    attackObject->AddComponent<CircleCollider>(circle, true);
  }
}

void AttackAnimation::RemoveHitbox()
{
  LOCK(weakAnimator, animator);

  auto attackObject = animator->gameObject.GetChild(attackObjectId);

  auto colliders = attackObject->GetComponents<Collider>();

  for (auto collider : colliders)
    attackObject->RemoveComponent(collider);
}

// === INNER LOOP ANIMATIONS

InnerLoopAnimation::InnerLoopAnimation(shared_ptr<Animator> animator) : AttackAnimation(animator)
{
  auto maybeRaisedOnSequenceStop = [this]()
  {
    // If this is the last phase
    if (sequencePhase == SequencePhase::PostLoop)
      OnSequenceStop.Invoke();

    else
    {
      LOCK(weakAnimator, animator);

      // Or the next animation is NOT this animation's next phase
      auto incomingAnimation = dynamic_pointer_cast<InnerLoopAnimation>(animator->GetIncomingAnimation());

      if (incomingAnimation == nullptr || incomingAnimation->pastPhaseId != id)
        OnSequenceStop.Invoke();
    }
  };

  OnStop.AddListener("InnerLoopAnimation::OnSequenceStop-check", maybeRaisedOnSequenceStop);

  if (QuitLoopOnInputRelease())
  {
    auto maybeChangeEndBehavior = [this]()
    {
      IF_LOCK(weakActionState, actionState)
      {
        if (sequencePhase == SequencePhase::InLoop && actionState->ActionInputReleased())
          endBehavior = CycleEndBehavior::PlayNext;
      }
    };

    OnCycleEnd.AddListener("update-inner-loop-end-behavior", maybeChangeEndBehavior);
  }
}

void InnerLoopAnimation::OnParentActionInputRelease()
{
  if (sequencePhase == SequencePhase::InLoop)
    endBehavior = CycleEndBehavior::PlayNext;
}

vector<AnimationFrame> InnerLoopAnimation::InitializeFrames()
{
  switch (sequencePhase)
  {
  case SequencePhase::PreLoop:
    return InitializePreLoopFrames();

  case SequencePhase::InLoop:
    return InitializeInLoopFrames();

  case SequencePhase::PostLoop:
    return InitializePostLoopFrames();
  }

  throw(runtime_error("Something is very wrong"));
}

shared_ptr<StatefulAnimation> InnerLoopAnimation::GetNextStateful()
{
  if (sequencePhase == SequencePhase::PostLoop)
    return nullptr;

  LOCK(weakAnimator, animator);

  // Build an instance of the first phase animation
  auto next = dynamic_pointer_cast<InnerLoopAnimation>(animator->BuildAnimation(Phase1Name()));

  // Give it our listeners
  next->OnCycleEnd.CopyListeners(OnCycleEnd);
  next->OnStop.CopyListeners(OnStop);
  next->OnSequenceStop.CopyListeners(OnSequenceStop);

  // Set it accordingly
  next->sequencePhase = SequencePhase(int(sequencePhase) + 1);
  next->pastPhaseId = id;

  // End behavior for in loop
  if (next->sequencePhase == SequencePhase::InLoop)
    next->endBehavior = CycleEndBehavior::Loop;

  // End behavior for post loop
  else
    next->endBehavior = CycleEndBehavior::PlayDefault;

  return next;
}

Animation::CycleEndBehavior &InnerLoopAnimation::EndBehavior() { return endBehavior; }

vector<AnimationFrame> InnerLoopAnimation::InitializePreLoopFrames() { return {}; }
vector<AnimationFrame> InnerLoopAnimation::InitializePostLoopFrames() { return {}; }

int InnerLoopAnimation::CancelFrame() const
{
  return sequencePhase == SequencePhase::PostLoop ? PostLoopCancelFrame() : -1;
}

int InnerLoopAnimation::OpenSequenceFrame() const
{
  return sequencePhase == SequencePhase::PostLoop ? PostLoopOpenSequenceFrame() : -1;
}

int InnerLoopAnimation::PostLoopCancelFrame() const { return -1; }

int InnerLoopAnimation::PostLoopOpenSequenceFrame() const { return -1; }

bool InnerLoopAnimation::QuitLoopOnInputRelease() const { return true; }

string InnerLoopAnimation::Name()
{
  switch (sequencePhase)
  {
  case SequencePhase::PreLoop:
    return Phase1Name();

  case SequencePhase::InLoop:
    return Phase2Name();

  case SequencePhase::PostLoop:
    return Phase3Name();
  }

  throw(runtime_error("Something is very wrong"));
}

string InnerLoopAnimation::Phase2Name() { return Phase1Name() + "InLoop"; }
string InnerLoopAnimation::Phase3Name() { return Phase1Name() + "PostLoop"; }