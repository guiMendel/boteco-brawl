#include "NewAnimationTypes.h"
#include "GameObject.h"
#include "CharacterStateManager.h"
#include "Attack.h"
#include "CircleCollider.h"

using namespace std;
using namespace Helper;

// === STATEFUL ANIMATIONS

StatefulAnimation::StatefulAnimation(Animator &animator) : Animation(animator) {}

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
        animator.gameObject.RequireComponent<CharacterStateManager>()->RemoveState(actionState->id);
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

Vector2 StatefulAnimation::GlobalVirtualPixelPosition(Vector2 virtualPixel)
{
  return GlobalVirtualPixelPosition(virtualPixel, GetFrame(currentFrame));
}
Vector2 StatefulAnimation::VirtualPixelPosition(Vector2 virtualPixel)
{
  return VirtualPixelPosition(virtualPixel, GetFrame(currentFrame));
}

Vector2 StatefulAnimation::GlobalVirtualPixelPosition(Vector2 virtualPixel, const AnimationFrame &frame)
{
  return VirtualPixelPosition(virtualPixel, frame) + animator.gameObject.GetPosition();
}

Vector2 StatefulAnimation::VirtualPixelPosition(Vector2 virtualPixel, const AnimationFrame &frame)
{
  // Get sprite renderer
  auto spriteRenderer = animator.gameObject.RequireComponent<SpriteRenderer>();

  // Get the frame's sprite
  auto sprite = frame.GetSprite();
  Assert(sprite != nullptr, "Provided frame must contain a sprite");

  // Global position of sprite's top-left pixel, in units
  Vector2 topLeftPosition = spriteRenderer->RenderPositionFor(animator.gameObject.GetPosition(), sprite);

  // Displacement to apply to object's position to get to top-left pixel's position
  Vector2 spriteOrigin = topLeftPosition - animator.gameObject.GetPosition();

  // When mirrored, we want to displace with reference to top-right pixel, so sum the sprite's width
  auto mirrorFactor = Vector2(GetSign(animator.gameObject.GetScale().x), 1);

  if (mirrorFactor.x < 0)
    spriteOrigin.x = spriteOrigin.x + sprite->GetWidth();

  return spriteOrigin + mirrorFactor * (virtualPixel + Vector2{0.5, 0.5}) / float(Game::defaultVirtualPixelsPerUnit);
}

// === ATTACK ANIMATIONS

float AttackAnimation::GetHitCooldown() const { return -1; }

DamageParameters AttackAnimation::GetAttackProperties() const { return {0, AttackImpulse(Vector2::Zero(), 0), 0}; }

void AttackAnimation::InternalOnStart() { SetupAttack(); }
void AttackAnimation::InternalOnStop()
{
  if (attackObjectId >= 0)
    animator.gameObject.RequireChild(attackObjectId)->RequestDestroy();
}

void AttackAnimation::SetupAttack()
{
  // Create child
  auto attackObject = animator.gameObject.CreateChild(ATTACK_OBJECT);
  attackObject->SetPhysicsLayer(PhysicsLayer::Hitbox);

  attackObjectId = attackObject->id;

  // Give it the attack component
  auto attack = attackObject->AddComponent<Attack>(GetAttackProperties(), GetHitCooldown());

  // React to connections
  attack->OnConnect.AddListener("animation-reaction", [this](shared_ptr<CharacterController> target)
                                { OnConnectAttack(target); });
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
  // Get attack object
  auto attackObject = animator.gameObject.RequireChild(attackObjectId);

  // First, remove all colliders already there
  RemoveHitbox();

  auto mirrorFactor = GetSign(attackObject->GetScale().x);

  // Now, add each provided area as a collider
  for (auto circle : hitboxAreas)
  {
    // Convert circle's virtual pixels to units
    circle = Circle(
        // But also remove mirroring because it will be reapplied on render
        VirtualPixelPosition(circle.center, frame) * mirrorFactor,
        circle.radius / float(Game::defaultVirtualPixelsPerUnit));

    attackObject->AddComponent<CircleCollider>(circle, true);
  }
}

void AttackAnimation::RemoveHitbox()
{

  auto attackObject = animator.gameObject.GetChild(attackObjectId);

  auto colliders = attackObject->GetComponents<Collider>();

  for (auto collider : colliders)
    attackObject->RemoveComponent(collider);
}

// === INNER LOOP ANIMATIONS

InnerLoopAnimation::InnerLoopAnimation(Animator &animator) : AttackAnimation(animator)
{
  auto weakAnimator{weak_ptr(dynamic_pointer_cast<Animator>(animator.GetShared()))};

  auto maybeRaisedOnSequenceStop = [this, weakAnimator]()
  {
    // If this is the last phase
    if (sequencePhase == SequencePhase::PostLoop)
      OnSequenceStop.Invoke();

    else
    {
      // Or the next animation is NOT this animation's next phase
      LOCK(weakAnimator, animator)
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
        if (
            // If in loop
            sequencePhase == SequencePhase::InLoop &&
            // And either action input was released
            (actionState->ActionInputReleased() ||
             // Or timed out
             (MaxInnerLoopDuration() >= 0 && GetInnerLoopElapsedTime() >= MaxInnerLoopDuration())))
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

  // Build an instance of the first phase animation
  auto next = dynamic_pointer_cast<InnerLoopAnimation>(animator.BuildAnimation(Phase1Name()));

  // Give it our listeners
  next->OnCycleEnd.CopyListeners(OnCycleEnd);
  next->OnStop.CopyListeners(OnStop);
  next->OnSequenceStop.CopyListeners(OnSequenceStop);

  // Set it accordingly
  next->sequencePhase = SequencePhase(int(sequencePhase) + 1);
  next->pastPhaseId = id;
  next->innerLoopElapsedTime = innerLoopElapsedTime;

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

void InnerLoopAnimation::OnUpdate(float deltaTime)
{
  if (sequencePhase != SequencePhase::InLoop)
    return;

  innerLoopElapsedTime += deltaTime;

  if (MaxInnerLoopDuration() >= 0 && innerLoopElapsedTime >= MaxInnerLoopDuration())
    Finish();
}

float InnerLoopAnimation::GetInnerLoopElapsedTime() const { return innerLoopElapsedTime; }

float InnerLoopAnimation::MaxInnerLoopDuration() const { return -1; }
