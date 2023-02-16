#include "CharacterKiba.h"
#include "KibaLemonAOE.h"
#include "CharacterKibaAnimations.h"
#include "Animator.h"
#include "CircleCollider.h"
#include "LandingAttackEffector.h"

using namespace std;
using namespace CharacterKibaAnimations;

CharacterKiba::CharacterKiba(GameObject &associatedObject)
    : Character(associatedObject) {}

static const CharacterKiba::transformerMap sequenceIndexTransformer{
    {"neutral", SequenceIndexTransformer::Repeat(3)}};

const CharacterKiba::transformerMap &CharacterKiba::GetSequenceIndexTransformer() const
{
  return sequenceIndexTransformer;
}

float CharacterKiba::GetBaseDamage() const { return 1; }
float CharacterKiba::GetDefaultArmor() const { return 1.2f; }
Rectangle CharacterKiba::GetHurtbox() const { return Rectangle({0, 0}, 1.8, 2); }
ColliderDensity CharacterKiba::GetDensity() const { return ColliderDensity::Character; }

float CharacterKiba::GetAcceleration() const { return 35; }
float CharacterKiba::GetDefaultSpeed() const { return 4; }

void CharacterKiba::AddAnimations(std::shared_ptr<Animator> animator) const
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
  animator->RegisterAnimation<SpecialHorizontal>();
  animator->RegisterAnimation<LandingAttack>();
  animator->RegisterAnimation<Crash>();
  animator->RegisterAnimation<Spin>();
}

void CharacterKiba::AddMechanics(std::shared_ptr<WorldObject> object) const
{
  // === LEMON AOE

  // Create lemon child
  auto lemonAoeObject = worldObject.CreateChild(LEMON_AOE_OBJECT);

  // Make it into a hitbox
  lemonAoeObject->SetPhysicsLayer(PhysicsLayer::Hitbox);

  // Give it the collider
  lemonAoeObject->AddComponent<CircleCollider>(Circle{3}, true);

  // Give it the AOE script
  lemonAoeObject->AddComponent<KibaLemonAOE>();

  // === LANDING ATTACKS

  // Decides whether the landing effector should trigger on land
  auto effectorCondition = [weakAnimator = weak_ptr(object->RequireComponent<Animator>())]()
  {
    LOCK(weakAnimator, animator);

    // Yes if the current animation is the shovel drop
    return animator->GetCurrentAnimation()->Name() == AIR_BODY_DROP_LOOP;
  };

  // Give it the landing attack effector
  object->AddComponent<LandingAttackEffector>(effectorCondition);
}

int CharacterKiba::GetDashRecoverFrame() const { return 1; }
