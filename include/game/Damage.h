#ifndef __DAMAGE__
#define __DAMAGE__

#include "Vector2.h"
#include <memory>

class GameObject;
class CircleCollider;

struct DamageParameters;

// Defines an attacks's impulse
struct AttackImpulse
{
  // For debugging
  friend class CircleCollider;
  
  // Zero impulse
  AttackImpulse();
  
  // Impulse with constant direction
  AttackImpulse(Vector2 direction, float magnitude);

  // Impulse away from origin
  AttackImpulse(std::shared_ptr<GameObject> origin, float magnitude);

  // Derive the impulse provided a target
  Vector2 DeriveImpulse(std::shared_ptr<GameObject> target) const;

  // Magnitude of impulse
  float magnitude;

  // Whether attack author is mirrored
  bool mirrored{false};

private:
  // Types of impulse
  enum Type
  {
    // Has a constant direction
    Directed,
    // Directs outwards from an origin point
    Repulsion
  };

  // Type of impulse
  Type type;

  // === TYPE SPECIFIC DATA

  // Directed
  Vector2 direction;

  // Repulsion
  std::weak_ptr<GameObject> weakOrigin;
};

// A struct that already derived impulse direction and heat damage from the attacker's parameters
struct Damage
{
  friend struct DamageParameters;

  // How much heat to add
  float heatDamage;

  // How much impulse to apply, along with direction
  // Corresponds to actual direction to which target will be launched
  AttackImpulse impulse;

  // How much time to leave target stunned, unable to move
  float stunTime;

  // Reference to object dealing this damage
  std::weak_ptr<GameObject> weakAuthor;

private:
  // Make constructor private
  Damage() = default;
};

// A struct that contains information on how to derive a Damage struct
struct DamageParameters
{
  // Damage applied to heat
  float heatDamage;

  // How much impulse to apply, along with direction
  // Right direction means aligned to facing direction
  AttackImpulse impulse;

  // How much time to leave target stunned, unable to move
  float stunTime;

  // Derive a damage struct
  Damage DeriveDamage(std::shared_ptr<GameObject> author) const;

  DamageParameters() = default;
};

#endif