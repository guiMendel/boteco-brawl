#ifndef __NEW_ANIMATION_TYPES__
#define __NEW_ANIMATION_TYPES__

#include "Animation.h"
#include "CharacterState.h"
#include "Damage.h"
#include "Circle.h"

// An animation that was created by an action and has a reference to the action's resulting state
class StatefulAnimation : public Animation
{
public:
  StatefulAnimation(std::shared_ptr<Animator> animator);
  virtual ~StatefulAnimation();

  // Register the state
  void RegisterState(std::shared_ptr<CharacterState> actionState);

protected:
  // Overtake responsibilty for this
  std::shared_ptr<Animation> GetNext() final override;

  // Provide new alternative
  virtual std::shared_ptr<StatefulAnimation> GetNextStateful();

  // At which frame this action's state is removed
  // A negative value means only when animation stops (default implementation)
  virtual int CancelFrame() const;

  // At which frame this action's state becomes open to sequences
  // A negative value means never (default implementation)
  virtual int OpenSequenceFrame() const;

  // Called when the linked state raises an OnActionInputRelease event
  virtual void OnParentActionInputRelease();

  std::weak_ptr<CharacterState> weakActionState;
};

// A stateful animation that specifically performs an attack with hitboxes
class AttackAnimation : public StatefulAnimation
{
public:
  AttackAnimation(std::shared_ptr<Animator> animator) : StatefulAnimation(animator) {}
  virtual ~AttackAnimation() {}

  virtual DamageParameters GetAttackProperties() const;

protected:
  // Sets hitbox for a given frame
  void FrameHitbox(AnimationFrame &frame, std::vector<Circle> hitboxAreas = {});

private:
  // Setup attack properties
  void InternalOnStart() override;
  void InternalOnStop() override;

  // Create attack child
  void SetupAttack();

  // Instantly give attack child a hitbox
  void SetHitbox(const AnimationFrame &frame, std::vector<Circle> hitboxAreas);

  // Remove hitbox from attack child
  void RemoveHitbox();

  // Id of attack object
  int attackObjectId{-1};
};

// An attack animation that has an inner loop (example: an animation that charges a punch for as long as a button is held down)
// It actually generates a dynamic animation that can provide 3 different sets of frames: pre-loop frames, in-loop frames and post-loop frames
class InnerLoopAnimation : public AttackAnimation
{
public:
  InnerLoopAnimation(std::shared_ptr<Animator> animator);
  virtual ~InnerLoopAnimation() {}

  // Raised when either the whole sequence ends or is interrupted
  Event OnSequenceStop;

protected:
  // Name of animations for each phase
  // The 2 last names can be left empty to be auto generated by appending "InLoop" and "PostLoop" to the first one
  virtual std::string Phase1Name() = 0;
  virtual std::string Phase2Name();
  virtual std::string Phase3Name();

  // Provides frames to be played before loop sequence
  // Default implementation provides no frames
  virtual std::vector<AnimationFrame> InitializePreLoopFrames();

  // Provides frames to be played during loop sequence
  virtual std::vector<AnimationFrame> InitializeInLoopFrames() = 0;

  // Provides frames to be played after loop sequence
  // Default implementation provides no frames
  virtual std::vector<AnimationFrame> InitializePostLoopFrames();

  // Provides cancel frame for last phase
  virtual int PostLoopCancelFrame() const;

  // Provides sequence open frame for last phase
  virtual int PostLoopOpenSequenceFrame() const;

  // Whether to automatically quit loop once state raised OnActionInputRelease
  // true by default
  virtual bool QuitLoopOnInputRelease() const;

  // Takes full responsibility over this method
  std::vector<AnimationFrame> InitializeFrames() final override;

  // Takes full responsibility over this method
  std::shared_ptr<StatefulAnimation> GetNextStateful() final override;

  // Takes full responsibility over this method
  CycleEndBehavior &EndBehavior() final override;

  // Takes full responsibility over this method
  int CancelFrame() const final override;

  // Takes full responsibility over this method
  int OpenSequenceFrame() const final override;

  // Takes full responsibility over this method
  std::string Name() final override;

  void OnParentActionInputRelease() override;

private:
  // Indicates for which phase of the complete pre-in-post sequence this animation is being instantiated for
  enum class SequencePhase
  {
    PreLoop,
    InLoop,
    PostLoop
  };

  // Store this animation's phase
  SequencePhase sequencePhase{SequencePhase::PreLoop};

  // Store the end behavior for this phase
  CycleEndBehavior endBehavior{CycleEndBehavior::PlayNext};

  // Store id of past phase
  int pastPhaseId{-1};
};

#endif