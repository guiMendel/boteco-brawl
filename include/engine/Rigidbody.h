#ifndef __RIGIDBODY__
#define __RIGIDBODY__

#include "Vector2.h"
#include "GameObject.h"
#include "Component.h"
#include "PhysicsSystem.h"
#include <unordered_set>

enum class RigidbodyType
{
  Static,
  Dynamic
};

class Collider;

class Rigidbody : public Component
{
  friend class PhysicsSystem;

public:
  Rigidbody(GameObject &associatedObject, RigidbodyType type, float elasticity = 0);

  virtual ~Rigidbody() {}

  void PhysicsUpdate(float deltaTime) override;

  void OnCollision(SatCollision::CollisionData collisionData) override;

  float GetMass() const;
  float GetInverseMass() const;

  void DynamicBodyUpdate(float deltaTime);

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

  // Whether collision with the given body happened THIS frame
  bool IsCollidingWith(Rigidbody &otherBody);

  // Whether collision with the given body happened last frame
  bool WasCollidingWith(Rigidbody &otherBody);

  // Whether to automatically calculate mass for this body
  bool useAutoMass{true};

  // Mass
  float mass{0};

  // Stores the inverse of mass
  float inverseMass{0};

  // Keeps track of all other bodies with which collision was detected on the current frame
  std::unordered_set<int> collidingBodies;

  // Holds the content of collidingBodies from the last frame
  std::unordered_set<int> oldCollidingBodies;
};

#endif