#include "CharacterKafta.h"
#include "CharacterKaftaAnimations.h"
#include "KaftaParry.h"
#include "Animator.h"
#include "LandingAttackEffector.h"

using namespace std;
using namespace CharacterKaftaAnimations;

CharacterKafta::CharacterKafta(GameObject &associatedObject)
    : Character(associatedObject) {}

static const CharacterKafta::transformerMap sequenceIndexTransformer{
    {"neutral", SequenceIndexTransformer::Repeat(3)},
    {"specialHorizontal", SequenceIndexTransformer::Repeat(2)}};

const CharacterKafta::transformerMap &CharacterKafta::GetSequenceIndexTransformer() const
{
  return sequenceIndexTransformer;
}

float CharacterKafta::GetBaseDamage() const { return 1; }
float CharacterKafta::GetDefaultArmor() const { return 1; }
Rectangle CharacterKafta::GetHurtbox() const { return Rectangle({0, 0}, 0.8, 2); }
ColliderDensity CharacterKafta::GetDensity() const { return ColliderDensity::Character; }

float CharacterKafta::GetAcceleration() const { return 45; }
float CharacterKafta::GetDefaultSpeed() const { return 7; }

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
  animator->RegisterAnimation<Neutral3>();
  animator->RegisterAnimation<Horizontal>();
  animator->RegisterAnimation<Up>();
  animator->RegisterAnimation<AirHorizontal>();
  animator->RegisterAnimation<AirUp>();
  animator->RegisterAnimation<AirDown>();
  animator->RegisterAnimation<SpecialNeutral>();
  animator->RegisterAnimation<SpecialHorizontal1>();
  animator->RegisterAnimation<SpecialHorizontal2>();
  animator->RegisterAnimation<Riposte>();
  animator->RegisterAnimation<Crash>();
  animator->RegisterAnimation<Spin>();
}

void CharacterKafta::AddMechanics(std::shared_ptr<WorldObject> object) const
{
  // Give it parry capacity
  object->AddComponent<KaftaParry>();
}

int CharacterKafta::GetDashRecoverFrame() const { return 1; }
