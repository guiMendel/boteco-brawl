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
#include "Tag.h"
#include "Timer.h"
#include "PhysicsSystem.h"

class GameState;

class GameObject
{
  friend class GameState;
  friend class PhysicsSystem;

public:
  // With dimensions
  GameObject(std::string name, Vector2 coordinates = Vector2(0, 0), double rotation = 0.0, std::shared_ptr<GameObject> parent = nullptr);
  ~GameObject();

  // Initialize
  void Awake();
  void Start();

  // Called once per frame
  void Update(float deltaTime);
  void PhysicsUpdate(float deltaTime);

  void OnStatePause();
  void OnStateResume();

  // Whether is dead
  bool DestroyRequested() const { return destroyRequested; }

  // Destroys the object
  void RequestDestroy() { destroyRequested = true; }

  // Adds a new component
  template <class T, typename... Args>
  auto AddComponent(Args &&...args) -> std::shared_ptr<T>
  {
    auto component = std::make_shared<T>(*this, std::forward<Args>(args)...);

    components.push_back(component);

    // Start it
    if (started)
    {
      component->RegisterToStateWithLayer();
      component->SafeStart();
    }

    return component;
  }

  // Removes an existing component
  void RemoveComponent(std::shared_ptr<Component> component);

  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto GetComponent() -> std::shared_ptr<T>
  {
    // Find the position of the component that is of the requested type
    auto componentIterator = std::find_if(
        components.begin(), components.end(), [](std::shared_ptr<Component> component)
        { return dynamic_cast<T *>(component.get()) != nullptr; });

    // Detect if not present
    if (componentIterator == components.end())
      return nullptr;

    return std::dynamic_pointer_cast<T>(*componentIterator);
  }

  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto GetComponents() -> std::vector<std::shared_ptr<T>>
  {
    std::vector<std::shared_ptr<T>> foundComponents;

    for (auto component : components)
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

  std::string GetName() const { return name; }

  std::shared_ptr<GameObject> CreateChild(std::string name);
  std::shared_ptr<GameObject> CreateChild(std::string name, Vector2 offset);
  std::shared_ptr<GameObject> CreateChild(std::string name, Vector2 offset, float offsetRotation);
  std::vector<std::shared_ptr<GameObject>> GetChildren();
  std::shared_ptr<GameObject> GetChild(std::string name);

  // Returns this object's shared pointer
  std::shared_ptr<GameObject> GetShared();

  // Get's pointer to parent, and ensures it's valid, unless this is the root object. If the parent is the root object, returns nullptr
  std::shared_ptr<GameObject> GetParent();

  // Set the parent
  void SetParent(std::shared_ptr<GameObject> newParent);

  // === ABSOLUTE VALUES

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

  void SetEnabled(bool enabled) { this->enabled = enabled; }
  bool IsEnabled() const { return enabled; }

  std::shared_ptr<GameState> GetState();

  // Whether to keep this object when loading next state (only works for root objects)
  void DontDestroyOnLoad(bool value = true);

  // Check if this gameObject is in the descendant lineage of the other object
  bool IsDescendant(GameObject &other);

  // A timer helper
  Timer timer;

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

  // Child objects
  std::unordered_map<int, std::weak_ptr<GameObject>> children;

private:
  // Initialize with given state
  GameObject(std::string name, int gameStateId, int id = -1);

  // Whether this is the root object
  bool IsRoot() const { return id == 0; }

  // Get's pointer to parent, and ensures it's valid, unless this is the root object
  std::shared_ptr<GameObject> InternalGetParent();

  // Unlinks from parent, destroys all children and destroys self
  void InternalDestroy();

  // Deletes reference to parent and paren't reference to self
  void UnlinkParent();

  // Announces collision to all components
  void OnCollision(Collision::CollisionData collisionData);
  void OnCollisionEnter(Collision::CollisionData collisionData);
  void OnCollisionExit(GameObject &other);

  // Announces trigger collision to all components
  void OnTriggerCollision(GameObject &other);
  void OnTriggerCollisionEnter(GameObject &other);
  void OnTriggerCollisionExit(GameObject &other);

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

  // Allows for registering to the state's variables
  void RegisterToState();

  void StartComponents();

  // Vector with all components of this object
  std::vector<std::shared_ptr<Component>> components;

  // State reference id
  int gameStateId;

  // Whether is dead
  bool destroyRequested{false};

  // Whether has already run started
  bool started{false};
  bool awoke{false};

  // Parent object
  std::weak_ptr<GameObject> weakParent;

  // The game object's name (not necessarily unique)
  std::string name;

  // Whether this object is enabled (updating & rendering)
  bool enabled{true};

  // Whether to keep this object when loading next state
  bool keepOnLoad{false};
};

#include "GameState.h"

#endif