#ifndef __RIGIDBODY__
#define __RIGIDBODY__

#include "Vector2.h"
#include "GameObject.h"
#include "Component.h"
#include "PhysicsSystem.h"
#include <unordered_set>

class Collider;

enum class RigidbodyType
{
  Static,
  Dynamic
};

class Rigidbody : public Component
{
  friend class PhysicsSystem;

public:
  static const float defaultAirFriction;

  Rigidbody(GameObject &associatedObject, RigidbodyType type, float elasticity = 0, float friction = 0.8f);

  virtual ~Rigidbody() {}

  void PhysicsUpdate(float deltaTime) override;
  void Render() override;
  RenderLayer GetRenderLayer() override { return RenderLayer::Debug; }

  void OnCollision(Collision::CollisionData collisionData) override;
  void OnTriggerCollision(GameObject &other) override;

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

  // Tells whether should use continuous detection in this frame
  bool ShouldUseContinuousDetection() const;

  // For bodies with continuous collision check, gets info for a rectangle that spans along it's trajectory since the last frame; as well as the trajectory angle
  std::pair<Rectangle, float> GetFrameTrajectory();

  RigidbodyType GetType() const;
  void SetType(RigidbodyType newType);

  // Returns whether a particle collides with any other body when cast from this object's position in some direction, over a fixed distance
  // Populates the raycast collision struct if a collision is detected
  bool Raycast(float angle, float maxDistance, RaycastCollisionData &data);
  bool Raycast(float angle, float maxDistance);

  // Returns whether this body's colliders collide with any other body when cast from this object's position in some direction, over a fixed distance
  // Populates the raycast collision struct if a collision is detected
  bool ColliderCast(float angle, float maxDistance, RaycastCollisionData &data, float scaleColliders = 1);
  bool ColliderCast(float angle, float maxDistance, float scaleColliders = 1);

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

  bool printIntersectionPoint{false};

  std::vector<Vector2> intersectionPoints;

  std::vector<std::pair<Vector2, Vector2>> printLines;

private:
  // Calculates mass based on colliders volumes & densities
  void DeriveMassFromColliders();

  void InternalSetMass(float newMass);

  // Whether collision with the given collider happened THIS frame
  bool IsCollidingWith(int id);
  bool IsCollidingWith(GameObject &other) { return IsCollidingWith(other.id); }

  // Whether collision with the given body happened last frame
  bool WasCollidingWith(int id);
  bool WasCollidingWith(GameObject &other) { return WasCollidingWith(other.id); }

  // Whether collision with the given collider happened THIS frame
  bool IsCollidingTriggerWith(int id);
  bool IsCollidingTriggerWith(GameObject &other) { return IsCollidingTriggerWith(other.id); }

  // Whether collision with the given body happened last frame
  bool WasCollidingTriggerWith(int id);
  bool WasCollidingTriggerWith(GameObject &other) { return WasCollidingTriggerWith(other.id); }

  // Calculates the value of the smallest dimension of it's colliders
  void CalculateSmallestColliderDimension();

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

  // Smallest dimension of this body's colliders
  float smallestDimension{0};
  float sqrSmallestDimension{0};

  bool printDebug{false};

  // Store this frame's result of the call to GetFrameTrajectory()
  std::pair<Rectangle, float> frameTrajectory;

  // Whether frameTrajectory is now outdated
  bool frameTrajectoryOutdated{true};

  // Keeps track of all other bodies with which collision was detected on the current frame
  std::unordered_set<int> collidingBodies;

  // Holds the content of collidingBodies from the last frame
  std::unordered_set<int> oldCollidingBodies;

  // Keeps track of all other bodies with which collision was detected on the current frame
  std::unordered_set<int> collidingTriggerBodies;

  // Holds the content of collidingTriggerBodies from the last frame
  std::unordered_set<int> oldCollidingTriggerBodies;
};

#endif