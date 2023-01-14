#ifndef __PHYSICS_SYSTEM__
#define __PHYSICS_SYSTEM__

#include "Vector2.h"
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include "Collision.h"
#include "PhysicsLayerHandler.h"
#include "TriggerCollisionData.h"

class GameState;
class Rigidbody;
class Collider;

// Stores data on a raycast collision
struct RaycastData
{
  // How long the ray was able to travel before collision
  float elapsedDistance;

  // The collider with which collision happened
  std::weak_ptr<Collider> other;
};

// Stores data on a collider cast collision
struct ColliderCastData
{
  // How long the collider structure was able to travel before collision
  float elapsedDistance;

  // Data on collision
  Collision::Data collision;

  // All detected trigger collisions
  std::vector<TriggerCollisionData> triggerCollisions;
};

// Stores data on how to filter a collision detection
struct CollisionFilter
{
  // Which objects should be ignored
  std::unordered_set<int> ignoredObjects;
};

class PhysicsSystem
{
  friend class GameState;
  friend class Rigidbody;

public:
  PhysicsSystem(GameState &gameState);

  // =================================
  // CONFIGURATION
  // =================================
public:
  // Initial gravity of the system
  static const Vector2 initialGravity;

  // Current gravity of the system
  Vector2 gravity{initialGravity};

  // =================================
  // FRAME EVENTS
  // =================================
public:
  void Update(float) {}
  void PhysicsUpdate(float);
  void Start() {}

  // =================================
  // COLLIDER STRUCTURES
  // =================================
public:
  // Adds a new collider to it's corresponding object entry
  void RegisterCollider(std::shared_ptr<Collider> collider, int objectId);

  // Deletes the colliders associated to this object ID
  void UnregisterColliders(int objectId);

private:
  using ValidatedColliders = std::vector<std::shared_ptr<Collider>>;
  using WeakColliders = std::vector<std::weak_ptr<Collider>>;

  // Pass each object through ValidateColliders and collect the results in a shuffled vector
  auto ValidateAllColliders(std::unordered_map<int, WeakColliders> &) -> std::vector<ValidatedColliders>;

  auto ValidateAllColliders(std::unordered_map<int, std::weak_ptr<Collider>> &) -> std::vector<std::shared_ptr<Collider>>;

  // For a specific object, removes any expired colliders from structure & returns the remaining ones as shared
  ValidatedColliders ValidateColliders(int id, WeakColliders &colliders);

  // Structure that maps each dynamic body object id to the list of it's colliders
  std::unordered_map<int, WeakColliders> dynamicColliderStructure;

  // Structure that maps each kinematic body object id to the list of it's colliders
  std::unordered_map<int, WeakColliders> kinematicColliderStructure;

  // Structure that maps each static body object id to the list of it's colliders
  std::unordered_map<int, WeakColliders> staticColliderStructure;

  // Structure that maps each trigger collider id to itself
  std::unordered_map<int, std::weak_ptr<Collider>> weakTriggerColliders;

  // =================================
  // PHYSICS OPERATIONS
  // =================================
public:
  // Applies a given friction to the velocity
  static Vector2 ApplyFriction(Vector2 velocity, float friction);

  // =================================
  // RAY & COLLIDER CASTING
  // =================================
public:
  // Returns whether a particle collides with any body when cast from the given position in some direction, over a fixed distance
  // Populates the raycast collision struct if a collision is detected
  // Allows filtering collisions with a CollisionFilter
  bool Raycast(Vector2 origin, float angle, float maxDistance, RaycastData &data, CollisionFilter filter = CollisionFilter());
  bool Raycast(Vector2 origin, float angle, float maxDistance, CollisionFilter filter = CollisionFilter());

  // Returns whether a group of colliders collides with any body when cast from the given position in some direction, over a fixed distance
  // Populates the raycast collision struct if a collision is detected
  // Allows filtering collisions with a CollisionFilter
  bool ColliderCast(std::vector<std::shared_ptr<Collider>> colliders, Vector2 origin, float angle, float maxDistance, ColliderCastData &data, CollisionFilter filter = CollisionFilter(), float colliderSizeScale = 1);
  bool ColliderCast(std::vector<std::shared_ptr<Collider>> colliders, Vector2 origin, float angle, float maxDistance, CollisionFilter filter = CollisionFilter(), float colliderSizeScale = 1);

private:
  // Returns whether detected a collision between the given particle and any bodies
  bool DetectRaycastCollisions(Vector2 particle, RaycastData &data, CollisionFilter filter);

  // Returns whether detected a collision between the given colliders and any bodies
  bool DetectColliderCastCollisions(std::vector<std::shared_ptr<Collider>> colliders, Vector2 position, ColliderCastData &data, CollisionFilter filter, float colliderSizeScale);

  // =================================
  // COLLISION DETECTION
  // =================================
private:
  // Detects all collisions (triggers included) and resolves them
  void HandleCollisions();

  // Normal collision detection for an object
  void DetectObjectCollisions(std::vector<ValidatedColliders>::iterator objectIterator, std::vector<ValidatedColliders>::iterator endIterator, std::vector<ValidatedColliders> &staticColliders);

  // Continuous collision detection for an object
  void DetectObjectBetweenFramesCollision(std::vector<ValidatedColliders>::iterator objectIterator);

  // Checks if there is collision between the two collider lists. If there is, populates the collisionData struct
  // Last 2 parameters are useful when this function is used by collider casts
  bool CheckForCollision(
      std::vector<std::shared_ptr<Collider>> colliders1, std::vector<std::shared_ptr<Collider>> colliders2, Collision::Data &collisionData, Vector2 displaceColliders1 = Vector2::Zero(), float scaleColliders1 = 1);

  // Applies impulse, checks if collision is entering & announces regular collision
  void ResolveCollision(Collision::Data collisionData);

  // Checks if collision is entering & announces regular collision
  void ResolveTriggerCollision(std::shared_ptr<Collider> collider1, std::shared_ptr<Collider> collider2);

  // This system's collision layer handler
  PhysicsLayerHandler layerHandler;

  // =================================
  // UTILITY
  // =================================
public:
  GameState &gameState;
};

#endif