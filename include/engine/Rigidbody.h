#ifndef __RIGIDBODY__
#define __RIGIDBODY__

#include "Vector2.h"
#include "GameObject.h"
#include "Component.h"

enum class RigidbodyType
{
  Static,
  Dynamic
};

class PhysicsSystem;
class Collider;

class Rigidbody : public Component
{
    friend class PhysicsSystem;
public:
  Rigidbody(GameObject &associatedObject, RigidbodyType type, float elasticity = 0.5f);

  virtual ~Rigidbody() {}

  void Update(float deltaTime) override;

  float GetMass() const { return mass; }
  float GetInverseMass() const { return inverseMass; }

  void SetMass(float newMass);

  // Gets the list of colliders associated with this body
  std::vector<std::shared_ptr<Collider>> GetColliders() const;

  // Sets whether to auto calculate mass from colliders
  void UseAutoMass(bool value);
  bool UsingAutoMass() const { return useAutoMass; }

  // Applies impulse to the body, altering it's velocity
  void ApplyImpulse(Vector2 impulse);

  // Velocity for each axis
  Vector2 velocity{0, 0};

  // What the type of this body is
  RigidbodyType type{RigidbodyType::Static};

  // Gravity modifier
  Vector2 gravityScale{1, 1};

  // Collision elasticity modifier (i.e. coefficient of restitution ε)
  float elasticity;

private:
  // Calculates mass based on colliders volumes & densities
  void DeriveMassFromColliders();

  void InternalSetMass(float newMass);

  // Whether to automatically calculate mass for this body
  bool useAutoMass{true};

  // Mass
  float mass{0};

  // Stores the inverse of mass
  float inverseMass{0};
};

#include "PhysicsSystem.h"
#include "Collider.h"

#endif