#include "CharacterKafta.h"
#include "CharacterKaftaAnimations.h"
#include "GunParry.h"
#include "Animator.h"
#include "LandingAttackEffector.h"

using namespace std;
using namespace CharacterKaftaAnimations;

CharacterKafta::CharacterKafta(GameObject &associatedObject)
    : Character(associatedObject) {}

static const CharacterKafta::transformerMap sequenceIndexTransformer{
    {"neutral", SequenceIndexTransformer::Repeat(2)}};

const CharacterKafta::transformerMap &CharacterKafta::GetSequenceIndexTransformer() const
{
  return sequenceIndexTransformer;
}

float CharacterKafta::GetBaseDamage() const { return 1; }
float CharacterKafta::GetDefaultArmor() const { return 1; }
Rectangle CharacterKafta::GetHurtbox() const { return Rectangle({0, 0}, 0.8, 1); }
ColliderDensity CharacterKafta::GetDensity() const { return ColliderDensity::Character; }

float CharacterKafta::GetAcceleration() const { return 35; }
float CharacterKafta::GetDefaultSpeed() const { return 5; }

void CharacterKafta::AddAnimations(std::shared_ptr<Animator> animator) const
{
  animator->RegisterAnimation<Idle>();
  animator->RegisterAnimation<Run>();
  animator->RegisterAnimation<Jump>();
  animator->RegisterAnimation<Rise>();
  animator->RegisterAnimation<Fall>();
  animator->RegisterAnimation<Land>();
  animator->RegisterAnimation<Brake>();
  animator->RegisterAnimation<Ouch1>();
  animator->RegisterAnimation<Ouch2>();
  animator->RegisterAnimation<Dash>();
  animator->RegisterAnimation<Neutral1>();
  animator->RegisterAnimation<Neutral2>();
  animator->RegisterAnimation<Horizontal>();
  animator->RegisterAnimation<Up>();
  animator->RegisterAnimation<AirHorizontal>();
  animator->RegisterAnimation<AirUp>();
  animator->RegisterAnimation<AirDown>();
  animator->RegisterAnimation<SpecialNeutral>();
  animator->RegisterAnimation<SpecialHorizontal>();
  animator->RegisterAnimation<Riposte>();
  animator->RegisterAnimation<LandingAttack>();
  animator->RegisterAnimation<Crash>();
  animator->RegisterAnimation<Spin>();
}

void CharacterKafta::AddMechanics(std::shared_ptr<WorldObject> object) const
{
  // Give it parry capacity
  object->AddComponent<GunParry>();

  // === LANDING ATTACKS

  // Decides whether the landing effector should trigger on land
  auto effectorCondition = [weakAnimator = weak_ptr(object->RequireComponent<Animator>())]()
  {
    LOCK(weakAnimator, animator);

    // Yes if the current animation is the shovel drop
    return animator->GetCurrentAnimation()->Name() == AIR_DOWN_SHOVEL_LOOP;
  };

  // Give it the landing attack effector
  object->AddComponent<LandingAttackEffector>(effectorCondition);
}
