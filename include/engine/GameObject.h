#ifndef __GAME_OBJECT__
#define __GAME_OBJECT__

#include <typeinfo>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <utility>
#include "Component.h"
#include "Vector2.h"
#include "Helper.h"
#include "PhysicsLayer.h"
#include "Tag.h"
#include "Timer.h"
#include "PhysicsSystem.h"
#include "TriggerCollisionData.h"

class GameState;

class GameObject
{
  friend class GameState;
  friend class PhysicsSystem;

  // Constructor dedicated for a state's root object
  // Initialize with given state
  GameObject(std::string name, int gameStateId, int id = -1);

public:
  // With dimensions
  GameObject(
      std::string name, Vector2 coordinates = Vector2(0, 0), double rotation = 0.0, std::shared_ptr<GameObject> parent = nullptr);
  ~GameObject();

  // =================================
  // FRAME EVENTS
  // =================================
public:
  // Initialize
  void Awake();
  void Start();

  // Called once per frame
  void Update(float deltaTime);
  void PhysicsUpdate(float deltaTime);

  void OnStatePause();
  void OnStateResume();

private:
  // Allows for registering to the state's variables
  void RegisterToState();

  // Whether has already run started
  bool started{false};
  bool awoke{false};

  // Id of last state to which this object has execute RegisterToState()
  int lastStateRegisteredTo{-1};

  // =================================
  // DESTRUCTION
  // =================================
public:
  // Whether is dead
  bool DestroyRequested() const { return destroyRequested; }

  // Destroys the object
  void RequestDestroy() { destroyRequested = true; }

  // Whether to keep this object when loading next state (only works for root objects)
  void DontDestroyOnLoad(bool value = true);

private:
  // Unlinks from parent, destroys all children and destroys self
  void InternalDestroy();

  // Deletes reference to parent and paren't reference to self
  void UnlinkParent();

  // Whether is dead
  bool destroyRequested{false};

  // Whether to keep this object when loading next state
  bool keepOnLoad{false};

  // =================================
  // COMPONENT HANDLING
  // =================================
private:
  // Map with all components of this object, indexed by the component's ids
  std::unordered_map<int, std::shared_ptr<Component>> components;

public:
  // Adds a new component
  template <class T, typename... Args>
  auto AddComponent(Args &&...args) -> std::shared_ptr<T>
  {
    auto component = std::make_shared<T>(*this, std::forward<Args>(args)...);

    components.insert({component->id, component});

    if (awoke)
      component->Awake();

    // Start it
    if (started)
    {
      component->RegisterToStateWithLayer();
      component->SafeStart();
    }

    return component;
  }

  // Removes an existing component
  decltype(components)::iterator RemoveComponent(std::shared_ptr<Component> component);

  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto GetComponent() -> std::shared_ptr<T>
  {
    // Find the position of the component that is of the requested type
    auto componentIterator = std::find_if(
        components.begin(), components.end(), [](std::pair<int, std::shared_ptr<Component>> componentEntry)
        { return dynamic_cast<T *>(componentEntry.second.get()) != nullptr; });

    // Detect if not present
    if (componentIterator == components.end())
      return nullptr;

    return std::dynamic_pointer_cast<T>(componentIterator->second);
  }

  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto GetComponents() -> std::vector<std::shared_ptr<T>>
  {
    std::vector<std::shared_ptr<T>> foundComponents;

    for (auto [componentId, component] : components)
    {
      if (dynamic_cast<T *>(component.get()) != nullptr)
        foundComponents.push_back(std::dynamic_pointer_cast<T>(component));
    }

    return foundComponents;
  }

  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto GetComponentsInChildren() -> std::vector<std::shared_ptr<T>>
  {
    return InternalGetComponentsInChildren<T>({});
  }

  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto GetComponentInChildren() -> std::shared_ptr<T>
  {
    // Try to get in this object
    auto component = GetComponent<T>();

    // If found, return it
    if (component != nullptr)
      return component;

    // For each child
    for (auto child : GetChildren())
    {
      auto component = child->GetComponentInChildren<T>();
      if (component != nullptr)
        return component;
    }

    return nullptr;
  }

  // Like GetComponentInChildren, but raises if it's not present
  template <class T>
  auto RequireComponentInChildren() -> std::shared_ptr<T>
  {
    auto component = GetComponentInChildren<T>();

    if (!component)
      throw std::runtime_error(std::string("Required component was not found in children.\nRequired component typeid name: ") + typeid(T).name());

    return component;
  }

  // Like GetComponent, but raises if it's not present
  template <class T>
  auto RequireComponent() -> std::shared_ptr<T>
  {
    auto component = GetComponent<T>();

    if (!component)
    {
      throw std::runtime_error(std::string("Required component was not found.\nRequired component typeid name: ") + typeid(T).name());
    }

    return component;
  }

  auto GetComponent(const Component *componentPointer) -> std::shared_ptr<Component>;
  auto RequireComponent(const Component *componentPointer) -> std::shared_ptr<Component>;

private:
  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto InternalGetComponentsInChildren(std::vector<std::shared_ptr<T>> foundComponents) -> std::vector<std::shared_ptr<T>>
  {
    // For each child
    for (auto child : GetChildren())
      foundComponents = child->InternalGetComponentsInChildren<T>(foundComponents);

    auto newComponents = GetComponents<T>();

    // Merge
    if (newComponents.size() > 0)
      foundComponents.insert(foundComponents.end(), newComponents.begin(), newComponents.end());

    return foundComponents;
  }

  // =================================
  // OBJECT PROPERTIES
  // =================================
public:
  std::string GetName() const { return name; }

  // Returns this object's shared pointer
  std::shared_ptr<GameObject> GetShared();

  // Where this object exists in game space, in absolute coordinates
  Vector2 GetPosition();
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

  void SetEnabled(bool enabled) { this->enabled = enabled; }
  bool IsEnabled() const;

  // Where this object exists in game space, relative to it's parent's position
  Vector2 localPosition;

  // Scale of the object
  Vector2 localScale{1, 1};

  // Object's rotation, in radians
  double localRotation{0};

  // Object's unique name
  const int id;

  // This object's tag
  Tag tag{Tag::None};

  // The game object's name (not necessarily unique)
  std::string name;

  // Whether this object is enabled (updating & rendering)
  bool enabled{true};

  // =================================
  // OBJECTS HIERARCHY
  // =================================
public:
  std::shared_ptr<GameObject> CreateChild(std::string name);
  std::shared_ptr<GameObject> CreateChild(std::string name, Vector2 offset);
  std::shared_ptr<GameObject> CreateChild(std::string name, Vector2 offset, float offsetRotation);
  std::vector<std::shared_ptr<GameObject>> GetChildren();
  std::shared_ptr<GameObject> GetChild(std::string name);
  std::shared_ptr<GameObject> GetChild(int id);
  std::shared_ptr<GameObject> RequireChild(std::string name);
  std::shared_ptr<GameObject> RequireChild(int id);

  // Get's pointer to parent, and ensures it's valid, unless this is the root object. If the parent is the root object, returns nullptr
  std::shared_ptr<GameObject> GetParent() const;

  // Set the parent
  void SetParent(std::shared_ptr<GameObject> newParent);

  // Check if this gameObject is in the descendant lineage of the other object
  bool IsDescendantOf(const GameObject &other) const;

  // Check if either object is a descendent of each other
  static bool SameLineage(const GameObject &first, const GameObject &second);

private:
  // Whether this is the root object
  bool IsRoot() const { return id == 0; }

  // Get's pointer to parent, and ensures it's valid, unless this is the root object
  std::shared_ptr<GameObject> InternalGetParent() const;

  // Child objects
  std::unordered_map<int, std::weak_ptr<GameObject>> children;

  // Parent object
  std::weak_ptr<GameObject> weakParent;

  // =================================
  // UTILITY
  // =================================
public:
  std::shared_ptr<GameState> GetState();

  // Allows for delaying a function execution
  // Returns a token id that can be used to cancel execution
  int DelayFunction(std::function<void()> procedure, float seconds);

  void CancelDelayedFunction(int tokenId);

  // A timer helper
  Timer timer;

private:
  // State reference id
  int gameStateId;

  // Functions currently waiting to be executed
  std::unordered_map<int, std::function<void()>> delayedFunctions;

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

private:
  // Verifies which collisions & triggers have exited this frame and raises them
  void DetectCollisionExits();

  // Should be called when the given component is about to be destroyed
  // Checks if the given component is a collider, and if so, raises all Exit messages on both sides
  void HandleColliderDestruction(std::shared_ptr<Component> component);

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

  // =================================
  // OPERATORS
  // =================================
public:
  bool operator==(const GameObject &other) const;

  explicit operator std::string() const;
};

std::ostream &operator<<(std::ostream &stream, const GameObject &vector);

#include "GameState.h"

#endif