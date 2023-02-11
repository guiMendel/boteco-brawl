#ifndef __CHARACTER__
#define __CHARACTER__

#include "WorldComponent.h"
#include "Animator.h"

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