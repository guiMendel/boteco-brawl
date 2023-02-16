#ifndef __CHARACTER__
#define __CHARACTER__

#include "WorldComponent.h"
#include "Animator.h"

#define SOUND_GRUNT_1 "grunt-1"
#define SOUND_GRUNT_2 "grunt-2"
#define SOUND_GRUNT_3 "grunt-3"
#define SOUND_GRUNT_4 "grunt-4"
#define SOUND_GRUNT_5 "grunt-5"
#define SOUND_GRUNT_6 "grunt-6"
#define SOUND_GRUNT_7 "grunt-7"
#define SOUND_GRUNT_8 "grunt-8"
#define SOUND_GRUNT_9 "grunt-9"
#define SOUND_GRUNT_10 "grunt-10"

#define SOUND_DEATH_1 "death-1"
#define SOUND_DEATH_2 "death-2"
#define SOUND_DEATH_3 "death-3"
#define SOUND_DEATH_4 "death-4"
#define SOUND_DEATH_5 "death-5"
#define SOUND_DEATH_6 "death-6"
#define SOUND_DEATH_7 "death-7"
#define SOUND_DEATH_8 "death-8"
#define SOUND_DEATH_9 "death-9"
#define SOUND_DEATH_10 "death-10"

// Base class for defining specific characteristics for each of the game's characters
class Character : public WorldComponent
{
public:
  Character(GameObject &associatedObject);
  virtual ~Character() {}

  // =================================
  // BASIC PROPERTIES
  // =================================
public:
  // === COMBAT

  // Base damage of this character
  virtual float GetBaseDamage() const = 0;

  // Default armor of character (all damage received is divided by this value before being applied)
  virtual float GetDefaultArmor() const = 0;

  // Hurtbox of this character
  virtual Rectangle GetHurtbox() const = 0;

  // Collider density of this character
  virtual ColliderDensity GetDensity() const = 0;

  // === MOVEMENT

  // Acceleration of movement
  virtual float GetAcceleration() const = 0;

  // Default speed of movement (max speed at which it moves under normal circumstances)
  virtual float GetDefaultSpeed() const = 0;

  // =================================
  // CHARACTER ANIMATIONS & MECHANICS
  // =================================
public:
  // Adds this character's animations to an animator
  virtual void AddAnimations(std::shared_ptr<Animator> animator) const = 0;

  // Adds character-specific mechanics to the character's object
  virtual void AddMechanics(std::shared_ptr<WorldObject> object) const = 0;

  // Get frame of dash at which point to switch from dash state to recovering state
  virtual int GetDashRecoverFrame() const = 0;

  // =================================
  // ATTACK ANIMATION SEQUENCE TRANSFORM
  // =================================
public:
  // Transforms an action repeat index into the corresponding animation index for it
  using indexTransformer = std::function<int(int)>;

  // Maps each action type to it's corresponding index transformer
  using transformerMap = std::unordered_map<std::string, indexTransformer>;

  // Transforms a sequence index for a given attack animation name
  int TransformSequenceIndexFor(std::string animation, int index) const;

protected:
  // Maps each attack animation name to a sequence index transformer function, which decides how to provide the sequence index number to the animator
  virtual const transformerMap &GetSequenceIndexTransformer() const = 0;
};

// Declares some useful sequence transformers creators
namespace SequenceIndexTransformer
{
  // Repeats a fixed number of animations
  Character::indexTransformer Repeat(int animations);

  // Starts looping an animation after it's reached
  Character::indexTransformer LoopSingleAfter(int loopAnimation);
}

#endif