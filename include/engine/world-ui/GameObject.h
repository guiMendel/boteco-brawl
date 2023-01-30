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

class GameScene;

// A type of data that can have Components as building blocks, a unique identifier, and a name
class GameObject
{
  friend class GameScene;

protected:
  // Constructor dedicated for a scene's root object
  // Initialize with given scene
  GameObject(std::string name, int gameSceneId, int id = -1);

public:
  // Default constructor
  GameObject(std::string name);

  virtual ~GameObject();

  // =================================
  // FRAME EVENTS
  // =================================
public:
  // Initialize
  virtual void Awake();
  virtual void Start();

  // Called once per frame
  virtual void Update(float deltaTime);
  virtual void PhysicsUpdate(float deltaTime);

  virtual void OnScenePause();
  virtual void OnSceneResume();

protected:
  // Allows for registering to the scene's variables
  virtual void RegisterToScene();

  // Whether has already run started
  bool started{false};
  bool awoke{false};

  // Id of last scene to which this object has executed RegisterToScene()
  int lastSceneRegisteredTo{-1};

  // =================================
  // DESTRUCTION
  // =================================
public:
  // Whether is dead
  bool DestroyRequested() const;

  // Destroys the object
  void RequestDestroy();

  // Whether to keep this object when loading next scene (only works for root objects)
  void DontDestroyOnLoad(bool value = true);

protected:
  // Destroys all components and destroys self
  virtual void InternalDestroy();

private:
  // Whether is dead
  bool destroyRequested{false};

  // Whether to keep this object when loading next scene
  bool keepOnLoad{false};

  // =================================
  // COMPONENT HANDLING
  // =================================
protected:
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
      component->RegisterToSceneWithLayer();
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

    return RequirePointerCast<T>(componentIterator->second);
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
        foundComponents.push_back(RequirePointerCast<T>(component));
    }

    return foundComponents;
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

  // =================================
  // OBJECT PROPERTIES
  // =================================
public:
  std::string GetName() const;

  // Where this object exists in space. The position units depend on actual implementation
  virtual Vector2 GetPosition() = 0;

  // Enable or disable and object
  void SetEnabled(bool enabled);

  // Whether this object is enabled or not (updating & rendering)
  bool IsEnabled() const;

  // Object's unique identifier
  const int id;

  // This object's tag
  Tag tag{Tag::None};

private:
  // The game object's name (not necessarily unique)
  std::string name;

  // Whether this object is enabled (updating & rendering)
  bool enabled{true};

  // =================================
  // OBJECT HIERARCHY
  // =================================
public:
  // Whether this is the root object
  bool IsRoot() const;

  // Executes the given function for this object and then cascades it down to any children it has
  virtual void CascadeDown(std::function<void(GameObject &)> callback, bool topDown = true) = 0;

protected:
  // Gets pointer to parent, and ensures it's valid. Raises when called from root object
  virtual std::shared_ptr<GameObject> InternalGetParent() const = 0;

  // =================================
  // UTILITY
  // =================================
public:
  // Returns reference to the scene this object exists in
  std::shared_ptr<GameScene> GetScene();

  // Allows for delaying a function execution
  // Returns a token id that can be used to cancel execution
  int DelayFunction(std::function<void()> procedure, float seconds);

  // Cancels the execution of a previously delayed function, given the token returned by DelayFunction
  void CancelDelayedFunction(int tokenId);

  // A timer helper. Stored values are increased each frame
  Timer timer;

private:
  // Trigger delayed functions whose timers are up
  void TriggerDelayedFunctions();

  // Scene reference id
  int gameSceneId;

  // Functions currently waiting to be executed (bool indicates if they are still supposed to be called)
  std::unordered_map<int, std::pair<std::function<void()>, bool>> delayedFunctions;

  // =================================
  // MODIFIERS
  // =================================
public:
  // Sets the delta time modifier for this object
  // It will be applied to delta times before they are passed down to components
  void SetTimeScale(float newScale);

  // What the current time scale is
  float GetTimeScale() const;

private:
  // Current value of time scale
  float localTimeScale{1};

  // =================================
  // OPERATORS
  // =================================
public:
  // Compares two object's. Will only return true if their id's are equal
  bool operator==(const GameObject &other) const;

  // Compares two object's. Will only return false if their id's are equal
  bool operator!=(const GameObject &other) const;

  // Returns s string structure which contains the object's id and name
  virtual explicit operator std::string() const;
};

// Calls the stirng cast
std::ostream &operator<<(std::ostream &stream, const GameObject &vector);

#endif