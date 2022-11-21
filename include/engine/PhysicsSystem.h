#ifndef __PHYSICS_SYSTEM__
#define __PHYSICS_SYSTEM__

#include "Vector2.h"
#include "Collider.h"
#include <vector>
#include <memory>
#include "SatCollision.h"

class GameState;
class Rigidbody;

class PhysicsSystem
{
  friend class GameState;
  friend class Rigidbody;

public:
  // Initial gravity of the system
  static const Vector2 initialGravity;

  PhysicsSystem(GameState &gameState);

  virtual void Update(float deltaTime);

  virtual void Render();

  virtual void Start();

  virtual void Pause();

  virtual void Resume();

  // Detects all collisions (triggers included) and resolves them
  void ResolveCollisions();

  // Adds a new collider to it's corresponding object entry
  void RegisterCollider(std::shared_ptr<Collider> collider, int objectId);

  // Current gravity of the system
  Vector2 gravity{initialGravity};

private:
  // Checks if there is collision between the two collider lists. If there is, populates the collisionData struct
  // Automatically raises trigger collisions
  bool CheckForCollision(
      std::vector<std::shared_ptr<Collider>> colliders1, std::vector<std::shared_ptr<Collider>> colliders2, SatCollision::CollisionData &collisionData);

  // Applies impulse to both bodies involved in the collision
  void ResolveCollision(SatCollision::CollisionData collisionData);

  // Pass each object through ValidateColliders and collect the results in a map
  std::unordered_map<int, std::vector<std::shared_ptr<Collider>>> ValidateAllColliders();

  // For a specific object, removes any expired colliders from structure & returns the remaining ones as shared
  std::vector<std::shared_ptr<Collider>> ValidateColliders(int id);

  // Structure that maps each object id to the list of it's colliders
  std::unordered_map<int, std::vector<std::weak_ptr<Collider>>> colliderStructure;

  GameState &gameState;
};

#include "Rigidbody.h"

#endif