#ifndef __WORLD_OBJECT__
#define __WORLD_OBJECT__

#include "GameObject.h"
#include "PhysicsLayer.h"
#include "PhysicsSystem.h"
#include "TriggerCollisionData.h"
#include "Parent.h"

class GameScene;

// A specific kind of GameObject which exists in the in-game world, has a scale, rotation, physical interactions, a parent and children World Objects
class WorldObject : public GameObject, public Parent<WorldObject>
{
  friend class GameScene;
  friend class PhysicsSystem;

  // Constructor dedicated for a scene's root object
  // Initialize with given scene
  WorldObject(std::string name, int gameSceneId, int id = -1);

public:
  // With properties
  WorldObject(
      std::string name, Vector2 coordinates = Vector2(0, 0), double rotation = 0.0, std::shared_ptr<WorldObject> parent = nullptr);

  virtual ~WorldObject();

  // =================================
  // FRAME EVENTS
  // =================================
public:
  // Add: Confine objects to limited game space
  void Update(float deltaTime) override;

  // Add: Detect collision exits
  void PhysicsUpdate(float deltaTime) override;

  // =================================
  // DESTRUCTION
  // =================================
public:
  // How far from origin an object can get in either coordinates before being destroyed
  static const float objectCollectionRange;

protected:
  // Calls DestroySelf and ignores returned value
  void InternalDestroy() override;

private:
  // Destroys children, calls GameObject::InternalDestroy, then unlinks from parent
  // Returns a valid iterator for the parent's new children after unlinking
  auto DestroySelf() -> std::unordered_map<int, std::weak_ptr<WorldObject>>::iterator;

  // Deletes reference to parent and parent's reference to self
  auto UnlinkParent() -> std::unordered_map<int, std::weak_ptr<WorldObject>>::iterator;

  // =================================
  // OBJECT PROPERTIES
  // =================================
public:
  // Returns this object's shared pointer
  std::shared_ptr<WorldObject> GetShared();

  // Where this object exists in game space relative to world origin, in units
  Vector2 GetPosition() override;
  void SetPosition(const Vector2 newPosition);
  void Translate(const Vector2 translation);

  // Absolute scale of the object
  Vector2 GetScale();
  void SetScale(const Vector2 newScale);

  // Absolute rotation in radians
  double GetRotation();
  void SetRotation(const double newRotation);

  // Sets a new value for the physics layer
  PhysicsLayer GetPhysicsLayer();
  void SetPhysicsLayer(PhysicsLayer);

  // Where this object exists in game space, relative to it's parent's position
  Vector2 localPosition;

  // Scale of the object relative to parent
  Vector2 localScale{1, 1};

  // Object's rotation relative to parent, in radians
  double localRotation{0};

  // =================================
  // OBJECTS HIERARCHY
  // =================================
public:
  std::shared_ptr<WorldObject> CreateChild(std::string name);
  std::shared_ptr<WorldObject> CreateChild(std::string name, Vector2 offset);
  std::shared_ptr<WorldObject> CreateChild(std::string name, Vector2 offset, float offsetRotation);

  // Get's pointer to parent, and ensures it's valid, unless this is the root object. If the parent is the root object, returns nullptr
  std::shared_ptr<WorldObject> GetParent() const;

  // Set the parent
  void SetParent(std::shared_ptr<WorldObject> newParent);

  // Check if this worldObject is in the descendant lineage of the other object
  bool IsDescendantOf(std::shared_ptr<WorldObject> other) const;

  // Check if either object is a descendent of each other
  static bool SameLineage(
      std::shared_ptr<WorldObject> first, std::shared_ptr<WorldObject> second);

  // Executes the given function for this object and then cascades it down to any children it has
  void CascadeDown(std::function<void(GameObject &)> callback, bool topDown = true) override;

protected:
  // Gets pointer to parent, and ensures it's valid. Raises when called from root object
  std::shared_ptr<GameObject> InternalGetParent() const override;

  // Gets pointer to parent, cast to world object
  std::shared_ptr<WorldObject> InternalGetWorldParent() const;

private:
  // Parent object
  std::weak_ptr<WorldObject> weakParent;

  // =================================
  // PHYSICS
  // =================================
public:
  // Announces collision to all components
  void OnCollision(Collision::Data collisionData);
  void OnCollisionEnter(Collision::Data collisionData);
  void OnCollisionExit(Collision::Data collisionData);

  // Whether collision with the given collider happened THIS frame
  bool IsCollidingWith(std::shared_ptr<Collider> collider);

  // Whether collision with the given body happened last frame
  bool WasCollidingWith(std::shared_ptr<Collider> collider);

  bool CollisionDealtWith(Collision::Data collisionData);
  bool CollisionDealtWithLastFrame(Collision::Data collisionData);

  // Announces trigger collision to all components
  void OnTriggerCollision(TriggerCollisionData triggerData);
  void OnTriggerCollisionEnter(TriggerCollisionData triggerData);
  void OnTriggerCollisionExit(TriggerCollisionData triggerData);

  // Whether collision with the given collider happened THIS frame
  bool IsTriggerCollidingWith(std::shared_ptr<Collider> collider);

  // Whether collision with the given body happened last frame
  bool WasTriggerCollidingWith(std::shared_ptr<Collider> collider);

  bool TriggerCollisionDealtWith(TriggerCollisionData triggerData);
  bool TriggerCollisionDealtWithLastFrame(TriggerCollisionData triggerData);

  // Raises all Exit messages on both sides of the collider's current interactions
  // Should be called when the given collider is about to be destroyed
  void HandleColliderDestruction(std::shared_ptr<Collider> collider);

private:
  // Verifies which collisions & triggers have exited this frame and raises them
  void DetectCollisionExits();

  // This object's physics layer
  PhysicsLayer physicsLayer{PhysicsLayer::None};

  // Whether current physics layer was the inherited layer
  bool inheritedPhysicsLayer{true};

  // Keeps track of all collisions registered this frame
  std::unordered_map<size_t, Collision::Data> frameCollisions;

  // Keeps track of all collisions registered last frame
  decltype(frameCollisions) lastFrameCollisions;

  // Keeps track of all triggers registered this frame
  std::unordered_map<size_t, TriggerCollisionData> frameTriggers;

  // Keeps track of all triggers registered last frame
  decltype(frameTriggers) lastFrameTriggers;
};

#include "GameScene.h"

#endif