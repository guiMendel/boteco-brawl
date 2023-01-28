#ifndef __CHARACTER__
#define __CHARACTER__

#include "Component.h"

// Base class for defining specific characteristics for each of the game's characters
class Character : public Component
{
public:
  using indexTransformer = std::function<int(int)>;
  using transformerMap = std::unordered_map<std::string, indexTransformer>;

  Character(WorldObject &associatedObject);
  virtual ~Character() {}

  // Transforms a sequence index for a given attack animation name
  int TransformSequenceIndexFor(std::string animation, int index) const;

  // Base damage of this character
  virtual float GetBaseDamage() const = 0;

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