#include "CharacterKiba.h"
#include "Sound.h"
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

  // === SOUNDS

  auto sound = object->RequireComponent<Sound>();

  sound->AddAudio(SOUND_GRUNT_1, "./assets/sounds/battle/grunts/kiba/damage_1_meghan.mp3");
  sound->AddAudio(SOUND_GRUNT_2, "./assets/sounds/battle/grunts/kiba/damage_2_meghan.mp3");
  sound->AddAudio(SOUND_GRUNT_3, "./assets/sounds/battle/grunts/kiba/damage_3_meghan.mp3");
  sound->AddAudio(SOUND_GRUNT_4, "./assets/sounds/battle/grunts/kiba/damage_4_meghan.mp3");
  sound->AddAudio(SOUND_GRUNT_5, "./assets/sounds/battle/grunts/kiba/damage_5_meghan.mp3");
  sound->AddAudio(SOUND_GRUNT_6, "./assets/sounds/battle/grunts/kiba/damage_6_meghan.mp3");
  sound->AddAudio(SOUND_GRUNT_7, "./assets/sounds/battle/grunts/kiba/damage_7_meghan.mp3");
  sound->AddAudio(SOUND_GRUNT_8, "./assets/sounds/battle/grunts/kiba/damage_8_meghan.mp3");
  sound->AddAudio(SOUND_GRUNT_9, "./assets/sounds/battle/grunts/kiba/damage_9_meghan.mp3");
  sound->AddAudio(SOUND_GRUNT_10, "./assets/sounds/battle/grunts/kiba/damage_10_meghan.mp3");

  sound->AddAudio(SOUND_DEATH_1, "./assets/sounds/battle/death/kiba/death_1_meghan.mp3");
  sound->AddAudio(SOUND_DEATH_2, "./assets/sounds/battle/death/kiba/death_2_meghan.mp3");
  sound->AddAudio(SOUND_DEATH_3, "./assets/sounds/battle/death/kiba/death_3_meghan.mp3");
  sound->AddAudio(SOUND_DEATH_4, "./assets/sounds/battle/death/kiba/death_4_meghan.mp3");
  sound->AddAudio(SOUND_DEATH_5, "./assets/sounds/battle/death/kiba/death_5_meghan.mp3");
  sound->AddAudio(SOUND_DEATH_6, "./assets/sounds/battle/death/kiba/death_6_meghan.mp3");
  sound->AddAudio(SOUND_DEATH_7, "./assets/sounds/battle/death/kiba/death_7_meghan.mp3");
  sound->AddAudio(SOUND_DEATH_8, "./assets/sounds/battle/death/kiba/death_8_meghan.mp3");
  sound->AddAudio(SOUND_DEATH_9, "./assets/sounds/battle/death/kiba/death_9_meghan.mp3");
  sound->AddAudio(SOUND_DEATH_10, "./assets/sounds/battle/death/kiba/death_10_meghan.mp3");
}

int CharacterKiba::GetDashRecoverFrame() const { return 1; }
