#ifndef __PHYSICS_SYSTEM__
#define __PHYSICS_SYSTEM__

#include "Vector2.h"
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include "SatCollision.h"

class GameState;
class Rigidbody;
class Collider;

// Stores data on a raycast collision
struct RaycastCollisionData
{
  // How long the ray was able to travel before collision
  float elapsedDistance;

  // The collider with which collision happened
  std::shared_ptr<Collider> other;
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
  // Initial gravity of the system
  static const Vector2 initialGravity;

  PhysicsSystem(GameState &gameState);

  void Update([[maybe_unused]] float deltaTime) {}
  void PhysicsUpdate([[maybe_unused]] float deltaTime);

  void Start() {}

  // Adds a new collider to it's corresponding object entry
  void RegisterCollider(std::shared_ptr<Collider> collider, int objectId);

  // Deletes the colliders associated to this object ID
  void UnregisterColliders(int objectId);

  // Applies a given friction to the velocity
  static Vector2 ApplyFriction(Vector2 velocity, float friction);

  // Returns whether a particle collides with any body when cast from the given position in some direction, over a fixed distance
  // Populates the raycast collision struct if a collision is detected
  // Allows filtering collisions with a CollisionFilter
  bool Raycast(Vector2 origin, float angle, float maxDistance, RaycastCollisionData &data, CollisionFilter filter = CollisionFilter());
  bool Raycast(Vector2 origin, float angle, float maxDistance, CollisionFilter filter = CollisionFilter());

  // Current gravity of the system
  Vector2 gravity{initialGravity};

private:
  using ValidatedColliders = std::vector<std::shared_ptr<Collider>>;
  using ValidatedCollidersMap = std::unordered_map<int, ValidatedColliders>;
  using WeakColliders = std::vector<std::weak_ptr<Collider>>;

  // Checks if there is collision between the two collider lists. If there is, populates the collisionData struct
  // Automatically raises trigger collisions
  bool CheckForCollision(
      std::vector<std::shared_ptr<Collider>> colliders1, std::vector<std::shared_ptr<Collider>> colliders2, Collision::CollisionData &collisionData);

  // Detects all collisions (triggers included) and resolves them
  void HandleCollisions();

  // Normal collision detection for an object
  void DetectCollisions(ValidatedCollidersMap::iterator objectIterator, ValidatedCollidersMap::iterator endIterator, ValidatedCollidersMap &staticColliders);

  // Continuous collision detection for an object
  void DetectBetweenFramesCollision(ValidatedCollidersMap::iterator objectIterator, ValidatedCollidersMap::iterator endIterator, ValidatedCollidersMap &staticColliders);

  // Returns whether detected a collision between the given particle and any bodies
  bool DetectCollisions(Vector2 particle, RaycastCollisionData &data, CollisionFilter filter);

  // Applies impulse, checks if collision is entering & announces regular collision
  void ResolveCollision(Collision::CollisionData collisionData);

  // Checks if collision is entering & announces regular collision
  void ResolveTriggerCollision(Rigidbody &body, Collider &collider);

  // Announces collision enter
  void EnterCollision(Collision::CollisionData collisionData);

  // Pass each object through ValidateColliders and collect the results in a map
  ValidatedCollidersMap ValidateAllColliders(std::unordered_map<int, WeakColliders> &);

  // For a specific object, removes any expired colliders from structure & returns the remaining ones as shared
  ValidatedColliders ValidateColliders(int id);

  // Finds the collider (if any) whose intersection with the trajectory rect is closest to the trajectory start
  // Returns a vector of the found colliders, the distance where this collision happened, and a callback to be executed if this collision is selected to be resolved
  // Automatically triggers any intersected Triggers
  auto FindTrajectoryIntersection(ValidatedColliders colliders, Rigidbody &sourceBody)
      -> std::tuple<ValidatedColliders, float, std::function<void()>>;

  // Finds an intersection between both bodies trajectories
  // Returns a vector of the first bodies colliders (empty if no intersection), the distance where this collision happened, and a callback to be executed if this collision is selected to be resolved
  auto FindTrajectoryIntersectionDouble(Rigidbody &otherBody, Rigidbody &sourceBody)
      -> std::tuple<ValidatedColliders, float, std::function<void()>>;

  // Structure that maps each dynamic body object id to the list of it's colliders
  std::unordered_map<int, WeakColliders> dynamicColliderStructure;

  // Structure that maps each static body object id to the list of it's colliders
  std::unordered_map<int, WeakColliders> staticColliderStructure;

  GameState &gameState;
};

#endif