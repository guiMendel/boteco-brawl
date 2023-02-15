#ifndef __CHARACTER_KIBA__
#define __CHARACTER_KIBA__

#include "Character.h"

#define LEMON_AOE_OBJECT "LemonAOE"

class CharacterKiba : public Character
{
public:
  CharacterKiba(GameObject &associatedObject);
  virtual ~CharacterKiba() {}

  // =================================
  // BASIC PROPERTIES
  // =================================
public:
  // === COMBAT

  // Base damage of this character
  float GetBaseDamage() const override;

  // Default armor of character (all damage received is divided by this value before being applied)
  float GetDefaultArmor() const override;

  // Hurtbox of this character
  Rectangle GetHurtbox() const override;

  // Collider density of this character
  ColliderDensity GetDensity() const override;

  // === MOVEMENT

  // Acceleration of movement
  float GetAcceleration() const override;

  // Default speed of movement (max speed at which it moves under normal circumstances)
  float GetDefaultSpeed() const override;

  // =================================
  // CHARACTER ANIMATIONS & MECHANICS
  // =================================
public:
  // Adds this character's animations to an animator
  void AddAnimations(std::shared_ptr<Animator> animator) const override;

  // Adds character-specific mechanics to the character's object
  void AddMechanics(std::shared_ptr<WorldObject> object) const override;

  // Get frame of dash at which point to switch from dash state to recovering state
  int GetDashRecoverFrame() const override;

  // =================================
  // ATTACK ANIMATION SEQUENCE TRANSFORM
  // =================================
protected:
  // Maps each attack animation name to a sequence index transformer function, which decides how to provide the sequence index number to the animator
  const transformerMap &GetSequenceIndexTransformer() const override;
};

#endif