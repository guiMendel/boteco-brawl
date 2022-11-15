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

class GameState;

class GameObject
{
  friend GameState;

public:
  // With dimensions
  GameObject(std::string name, Vector2 coordinates = Vector2(0, 0), double rotation = 0.0, std::shared_ptr<GameObject> parent = nullptr);
  ~GameObject();

  // Initialize
  void Start();

  // Called once per frame
  void Update(float deltaTime);

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
      component->StartAndRegisterLayer();

    return component;
  }

  // Removes an existing component
  void RemoveComponent(std::shared_ptr<Component> component);

  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto GetComponent() const -> std::shared_ptr<T>
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

  // Like GetComponent, but raises if it's not present
  template <class T>
  auto RequireComponent() const -> std::shared_ptr<T>
  {
    auto component = GetComponent<T>();

    if (!component)
    {
      throw std::runtime_error(std::string("Required component was not found.\nRequired component typeid name: ") + typeid(T).name());
    }

    return component;
  }

  auto GetComponent(const Component *componentPointer) const -> std::shared_ptr<Component>;

  std::string GetName() const { return name; }

  std::vector<std::shared_ptr<GameObject>> GetChildren();

  // Returns this object's shared pointer
  std::shared_ptr<GameObject> GetShared() const;

  // Get's pointer to parent, and ensures it's valid, unless this is the root object. If the parent is the root object, returns nullptr
  std::shared_ptr<GameObject> GetParent() const;

  // Set the parent
  void SetParent(std::shared_ptr<GameObject> newParent);

  // === ABSOLUTE VALUES

  // Where this object exists in game space, in absolute coordinates
  Vector2 GetPosition() const;
  void SetPosition(const Vector2 newPosition);

  // Absolute scale of the object
  Vector2 GetScale() const;
  void SetScale(const Vector2 newScale);

  // Absolute rotation in radians
  double GetRotation() const;
  void SetRotation(const double newRotation);

  void SetEnabled(bool enabled) { this->enabled = enabled; }
  bool IsEnabled() const { return enabled; }

  // A timer helper
  Timer timer;

  // State reference
  GameState &gameState;

  // Where this object exists in game space, relative to it's parent's position
  Vector2 localPosition;

  // Scale of the object
  Vector2 localScale{1, 1};

  // Object's rotation, in radians
  double localRotation{0};

  // Object's unique identifier
  const int id;

  // This object's tag
  Tag tag{Tag::None};

  // Child objects
  std::unordered_map<int, std::weak_ptr<GameObject>> children;

private:
  // Initialize with given state
  GameObject(std::string name, GameState &gameState);

  // Whether this is the root object
  bool IsRoot() const { return id == 0; }

  // Get's pointer to parent, and ensures it's valid, unless this is the root object
  std::shared_ptr<GameObject> InternalGetParent() const;

  // Unlinks from parent, destroys all children and destroys self
  void InternalDestroy();

  // Deletes reference to parent and paren't reference to self
  void UnlinkParent();

  // Announces collision to all components
  void OnCollision(GameObject &other);

  // Vector with all components of this object
  std::vector<std::shared_ptr<Component>> components;

  // Whether is dead
  bool destroyRequested{false};

  // Whether has already run started
  bool started{false};

  // Parent object
  std::weak_ptr<GameObject> weakParent;

  // The game object's name (not necessarily unique)
  std::string name;

  // Whether this object is enabled (updating & rendering)
  bool enabled{true};
};

#include "GameState.h"

#endif