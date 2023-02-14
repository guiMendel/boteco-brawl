#ifndef __CHARACTER_ANIMATIONS__
#define __CHARACTER_ANIMATIONS__

#include <memory>
#include "NewAnimationTypes.h"
#include "Animator.h"

// === HELPERS

#define CONSTRUCTOR_AND_DESTRUCTOR_WITH_PARENT(ClassName, Parent) \
  ClassName(Animator &animator) : Parent(animator)                \
  {                                                               \
  }                                                               \
  virtual ~ClassName()                                            \
  {                                                               \
  }

#define CONSTRUCTOR_AND_DESTRUCTOR(ClassName) CONSTRUCTOR_AND_DESTRUCTOR_WITH_PARENT(ClassName, StatefulAnimation)
#define ATTACK_CONSTRUCTOR_AND_DESTRUCTOR(ClassName) CONSTRUCTOR_AND_DESTRUCTOR_WITH_PARENT(ClassName, AttackAnimation)
#define LOOP_CONSTRUCTOR_AND_DESTRUCTOR(ClassName) CONSTRUCTOR_AND_DESTRUCTOR_WITH_PARENT(ClassName, InnerLoopAnimation)

#define FIELD(Type, Name, initialValue) \
  Type &Name() override                 \
  {                                     \
    static Type field{initialValue};    \
    return field;                       \
  }

#define DEF_FRAMES(frames)                                \
  std::vector<AnimationFrame> InitializeFrames() override \
  {                                                       \
    return frames;                                        \
  }

#define DELCARE_FRAMES std::vector<AnimationFrame> InitializeFrames() override;

#define DEF_NAME(name)        \
  std::string Name() override \
  {                           \
    return name;              \
  }

#define DEF_FIRST_NAME(name)        \
  std::string Phase1Name() override \
  {                                 \
    return name;                    \
  }

#define DECLARE(Type, Name) Type &Name() override;

#define SET_DAMAGE(damage, impulse, stunTime)           \
  DamageParameters GetAttackProperties() const override \
  {                                                     \
    return {damage, impulse, stunTime};                 \
  }

#define SET_HIT_COOLDOWN(cooldown)      \
  float GetHitCooldown() const override \
  {                                     \
    return cooldown;                    \
  }

#define ATTACK_SEQUENCE(frame)           \
  int OpenSequenceFrame() const override \
  {                                      \
    return frame;                        \
  }

#define ATTACK_CANCEL(frame)       \
  int CancelFrame() const override \
  {                                \
    return frame;                  \
  }

#define BASE_DAMAGE animator.worldObject.RequireComponent<Character>()->GetBaseDamage()

namespace CharacterAnimationHelper
{
  // Takes the last frame of a sequence, replicates it n - 1 times, and sets all of it's instances duration
  void SplitLastFrame(std::vector<AnimationFrame> &frames, int numberOfInstances, float newDuration);

  // Applies shaking effect to the loop cycle of an InnerLoop animation type
  void ShakeLoop(const InnerLoopAnimation &animation);

  // Returns a callback to stop a shake initiated by the ShakeLoop method
  std::function<void(WorldObject &)> StopShakeCallback();

  // Returns a callback that displaces the character the given amount in units
  std::function<void(WorldObject &)> DisplaceCallback(Vector2 displacement);

  // Returns a callback that resets the attacks's hit targets
  std::function<void(WorldObject &)> ResetHitTargetsCallback();

  // Returns a callback that enables a parry
  template <class T>
  std::function<void(WorldObject &)> EnableParryCallback()
  {
    return [](WorldObject &target)
    {
      target.ComponentOwner::RequireComponent<T>()->SetParry(true);
    };
  }

  // Returns a callback that disables a parry
  template <class T>
  std::function<void(WorldObject &)> DisableParryCallback()
  {
    return [](WorldObject &target)
    {
      target.ComponentOwner::RequireComponent<T>()->SetParry(false);
    };
  }

}

#endif