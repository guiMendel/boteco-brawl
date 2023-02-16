#include "CharacterKafta.h"
#include "Sound.h"
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

  // === SOUNDS

  auto sound = object->RequireComponent<Sound>();

  sound->AddAudio(SOUND_GRUNT_1, "./assets/sounds/battle/grunts/kafta/damage_1_sean.mp3");
  sound->AddAudio(SOUND_GRUNT_2, "./assets/sounds/battle/grunts/kafta/damage_2_sean.mp3");
  sound->AddAudio(SOUND_GRUNT_3, "./assets/sounds/battle/grunts/kafta/damage_3_sean.mp3");
  sound->AddAudio(SOUND_GRUNT_4, "./assets/sounds/battle/grunts/kafta/damage_4_sean.mp3");
  sound->AddAudio(SOUND_GRUNT_5, "./assets/sounds/battle/grunts/kafta/damage_5_sean.mp3");
  sound->AddAudio(SOUND_GRUNT_6, "./assets/sounds/battle/grunts/kafta/damage_6_sean.mp3");
  sound->AddAudio(SOUND_GRUNT_7, "./assets/sounds/battle/grunts/kafta/damage_7_sean.mp3");
  sound->AddAudio(SOUND_GRUNT_8, "./assets/sounds/battle/grunts/kafta/damage_8_sean.mp3");
  sound->AddAudio(SOUND_GRUNT_9, "./assets/sounds/battle/grunts/kafta/damage_9_sean.mp3");
  sound->AddAudio(SOUND_GRUNT_10, "./assets/sounds/battle/grunts/kafta/damage_10_sean.mp3");

  sound->AddAudio(SOUND_DEATH_1, "./assets/sounds/battle/death/kafta/death_1_sean.mp3");
  sound->AddAudio(SOUND_DEATH_2, "./assets/sounds/battle/death/kafta/death_2_sean.mp3");
  sound->AddAudio(SOUND_DEATH_3, "./assets/sounds/battle/death/kafta/death_3_sean.mp3");
  sound->AddAudio(SOUND_DEATH_4, "./assets/sounds/battle/death/kafta/death_4_sean.mp3");
  sound->AddAudio(SOUND_DEATH_5, "./assets/sounds/battle/death/kafta/death_5_sean.mp3");
  sound->AddAudio(SOUND_DEATH_6, "./assets/sounds/battle/death/kafta/death_6_sean.mp3");
  sound->AddAudio(SOUND_DEATH_7, "./assets/sounds/battle/death/kafta/death_7_sean.mp3");
  sound->AddAudio(SOUND_DEATH_8, "./assets/sounds/battle/death/kafta/death_8_sean.mp3");
  sound->AddAudio(SOUND_DEATH_9, "./assets/sounds/battle/death/kafta/death_9_sean.mp3");
  sound->AddAudio(SOUND_DEATH_10, "./assets/sounds/battle/death/kafta/death_10_sean.mp3");
}

int CharacterKafta::GetDashRecoverFrame() const { return 1; }
