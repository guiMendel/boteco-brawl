#ifndef __RIGIDBODY__
#define __RIGIDBODY__

#include "Vector2.h"
#include "WorldObject.h"
#include "WorldComponent.h"
#include "PhysicsSystem.h"
#include <unordered_set>

class Collider;

enum class RigidbodyType
{
  Static,
  Kinematic,
  Dynamic
};

class Rigidbody : public WorldComponent
{
  friend class PhysicsSystem;

public:
  static const float defaultAirFriction;

  Rigidbody(GameObject &associatedObject, RigidbodyType type, float elasticity = 0, float friction = 0.8f);

  virtual ~Rigidbody() {}

  void PhysicsUpdate(float deltaTime) override;
  void Render() override;
  RenderLayer GetRenderLayer() override { return RenderLayer::Debug; }

  float GetMass() const;
  float GetInverseMass() const;

  void DynamicBodyUpdate(float deltaTime);
  void KinematicBodyUpdate(float deltaTime);

  void SetMass(float newMass);

  // Gets the list of colliders associated with this body
  std::vector<std::shared_ptr<Collider>> GetColliders() const;

  // Sets whether to auto calculate mass from colliders
  void UseAutoMass(bool value);
  bool UsingAutoMass() const { return useAutoMass; }

  // Applies impulse to the body, altering it's velocity
  void ApplyImpulse(Vector2 impulse);

  // Tells whether should use continuous detection in this frame
  bool ShouldUseContinuousDetection() const;

  // For bodies with continuous collision check, gets trajectory from last frame to this frame
  Vector2 GetFrameTrajectory();

  RigidbodyType GetType() const;
  void SetType(RigidbodyType newType);

  // Returns whether a particle collides with any other body when cast from this object's position in some direction, over a fixed distance
  // Populates the raycast collision struct if a collision is detected
  bool Raycast(float angle, float maxDistance, RaycastData &data);
  bool Raycast(float angle, float maxDistance);

  // Returns whether this body's colliders collide with any other body when cast from this object's position in some direction, over a fixed distance
  // Populates the raycast collision struct if a collision is detected
  bool ColliderCast(float angle, float maxDistance, ColliderCastData &data, float scaleColliders = 1);
  bool ColliderCast(float angle, float maxDistance, float scaleColliders = 1);

  bool IsStatic() const;
  bool IsKinematic() const;
  bool IsDynamic() const;

  // Get min dimension of all colliders
  float GetMinDimension() const;

  // Velocity for each axis
  Vector2 velocity{0, 0};

  // Gravity modifier
  Vector2 gravityScale{1, 1};

  // Whether this body will check each frame if a collision should have happened in between frames
  bool continuousCollisions{false};

  // Collision elasticity modifier (i.e. coefficient of restitution ε)
  float elasticity;

  // Friction applied on each collision. Is a modifier applied to speed per second
  float friction;

  // Friction applied on each frame. Is a modifier applied to speed per second
  float airFriction{defaultAirFriction};

private:
  // Calculates mass based on colliders volumes & densities
  void DeriveMassFromColliders();

  void InternalSetMass(float newMass);

  // What the type of this body is
  RigidbodyType type{RigidbodyType::Static};

  // Whether to automatically calculate mass for this body
  bool useAutoMass{true};

  // Mass
  float mass{0};

  // Stores the inverse of mass
  float inverseMass{0};

  // Stores the last position of the body
  Vector2 lastPosition;
};

#endif