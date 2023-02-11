#include "CharacterTest.h"
#include "CharacterTestAnimations.h"
#include "GunParry.h"
#include "Animator.h"
#include "LandingAttackEffector.h"

using namespace std;
using namespace CharacterTestAnimations;

CharacterTest::CharacterTest(GameObject &associatedObject)
    : Character(associatedObject) {}

static const CharacterTest::transformerMap sequenceIndexTransformer{
    {"neutral", SequenceIndexTransformer::Repeat(2)}};

const CharacterTest::transformerMap &CharacterTest::GetSequenceIndexTransformer() const
{
  return sequenceIndexTransformer;
}

float CharacterTest::GetBaseDamage() const { return 1; }
float CharacterTest::GetDefaultArmor() const { return 1; }
Rectangle CharacterTest::GetHurtbox() const { return Rectangle({0, 0}, 0.8, 1); }
ColliderDensity CharacterTest::GetDensity() const { return ColliderDensity::Character; }

float CharacterTest::GetAcceleration() const { return 35; }
float CharacterTest::GetDefaultSpeed() const { return 5; }

void CharacterTest::AddAnimations(std::shared_ptr<Animator> animator) const
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

void CharacterTest::AddMechanics(std::shared_ptr<WorldObject> object) const
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
