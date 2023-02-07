#ifndef __GAME_SCENE__
#define __GAME_SCENE__

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <list>
#include <SDL.h>
#include "GameObject.h"
#include "WorldObject.h"
#include "SpriteRenderer.h"
#include "Music.h"
#include "InputManager.h"
#include "Vector2.h"
#include "PhysicsSystem.h"
#include "ParticleSystem.h"
#include "Renderable.h"

class Component;
class Collider;
class Camera;
class Game;

// Abstract class that defines a scene of the game
class GameScene
{
  friend class Collider;
  friend class Camera;
  friend class GameObject;
  friend class WorldObject;
  friend class Game;

public:
  GameScene();

  virtual ~GameScene();

  // =================================
  // FRAME EVENTS
  // =================================
public:
  void Update(float deltaTime);

  virtual void PhysicsUpdate(float deltaTime);

  virtual void Render();

  virtual void Start();

  virtual void Pause();

  virtual void Resume();

protected:
  // Allows for child class scenes to hook behavior to the update loop
  virtual void OnUpdate(float) {}

  // Whether the scene has executed the start method
  bool started{false};

  // Whether the scene has executed the awake method
  bool awoke{false};

  // =================================
  // SCENE PROPERTIES
  // =================================
public:
  // The scene's own physics system instance
  PhysicsSystem physicsSystem;

  // Particle system
  ParticleSystem particleSystem;

  // A timer helper
  Timer timer;

  // Unique identifier of this scene
  const int id;

protected:
  // Reference to input manager
  InputManager &inputManager;

  // =================================
  // GAME SCENE HANDLING
  // =================================
public:
  // Called right before scene is popped
  void Destroy();

  // Whether to remove this scene from the queue
  bool PopRequested() { return popRequested; }

  // Whether the game should exit
  bool QuitRequested() { return quitRequested; }

protected:
  // Indicates that the scene mus tbe removed from queue
  bool popRequested{false};

  // Indicates that the game must exit
  bool quitRequested{false};

  // =================================
  // SCENE'S GAME OBJECTS
  // =================================
public:
  // Registers an object to this scene
  std::shared_ptr<GameObject> RegisterObject(GameObject *gameObject);

  // Registers an object to this scene
  std::shared_ptr<GameObject> RegisterObject(std::shared_ptr<GameObject> gameObject);

  // Removes an object given it's id
  void RemoveObject(int id);

  // Removes an object given it's shared pointer
  void RemoveObject(std::shared_ptr<GameObject> gameObject);

  // Creates an empty game object and registers it to the scene
  template <class T, typename... Args>
  std::shared_ptr<T> NewObject(Args &&...args)
  {
    // Create the object, which automatically registers itself to the scene
    auto object = std::make_shared<T>(std::forward<Args>(args)...);

    // Request it's registration
    RegisterObject(object);

    return object;
  }

  // Creates a new world object using a recipe
  template <typename... Args>
  std::shared_ptr<WorldObject> Instantiate(
      std::string name, std::function<void(std::shared_ptr<WorldObject>)> recipe = nullptr, Args &&...args)
  {
    // Create the object, which automatically registers itself to the scene
    auto object = NewObject<WorldObject>(name, std::forward<Args>(args)...);

    // Initialize it
    if (recipe)
      recipe(object);

    return object;
  }

  // Gets a game object by it's id
  std::shared_ptr<GameObject> GetGameObject(int id);

  // Finds an object by it's name
  std::shared_ptr<GameObject> GetGameObject(std::string name);

  // Throws if this object doesn't exist
  std::shared_ptr<GameObject> RequireGameObject(int id);

  // Throws if this object doesn't exist
  std::shared_ptr<GameObject> RequireGameObject(std::string name);

  // Gets a world object by it's id
  std::shared_ptr<WorldObject> GetWorldObject(int id);

  // Gets a world object by it's name
  std::shared_ptr<WorldObject> GetWorldObject(std::string name);

  // Throws if this world object doesn't exist
  std::shared_ptr<WorldObject> RequireWorldObject(int id);

  // Throws if this world object doesn't exist
  std::shared_ptr<WorldObject> RequireWorldObject(std::string name);

  // Gets a ui object by it's id. Throws if cast to the provided class fails.
  template <class T>
  std::shared_ptr<T> GetUIObject(int id)
  {
    return std::dynamic_pointer_cast<T>(GetGameObject(id));
  }

  // Gets a ui object by it's name. Throws if cast to the provided class fails.
  template <class T>
  std::shared_ptr<T> GetUIObject(std::string name)
  {
    return std::dynamic_pointer_cast<T>(GetGameObject(name));
  }

  // Throws if this ui object doesn't exist. Throws if cast to the provided class fails.
  template <class T>
  std::shared_ptr<T> RequireUIObject(int id) { return RequirePointerCast<T>(RequireGameObject(id)); }

  // Throws if this ui object doesn't exist. Throws if cast to the provided class fails.
  template <class T>
  std::shared_ptr<T> RequireUIObject(std::string name) { return RequirePointerCast<T>(RequireGameObject(name)); }

  // Finds a component in this scene's hierarchy
  template <class T>
  auto FindComponent() -> std::shared_ptr<T>
  {
    // Find the position of the object that is of the requested type
    for (auto &objectPair : gameObjects)
    {
      auto component = objectPair.second->GetComponent<T>();

      if (component != nullptr)
        return component;
    }

    return nullptr;
  }

  // Finds a component in this scene's hierarchy and throws if it's not found
  template <class T>
  auto RequireFindComponent() -> std::shared_ptr<T>
  {
    auto object = FindComponent<T>();

    Helper::Assert(object != nullptr, "Required object type was not present in game scene");

    return object;
  }

  // Initializes the scene's objects
  virtual void InitializeObjects() = 0;

protected:
  // All of the scene's objects
  std::unordered_map<int, std::shared_ptr<GameObject>> gameObjects;

  // Root object reference
  std::shared_ptr<WorldObject> rootObject;

private:
  // Gets the root object
  std::shared_ptr<WorldObject> GetRootObject();

  // Gets all objects which must be carried on to the next scene, and removes them from this scene (without destroying them)
  std::vector<std::shared_ptr<GameObject>> ExtractObjectsToCarryOn();

  // Executes this function for each object, cascading down the hierarchy
  void CascadeDown(std::function<void(GameObject &)> callback, bool topDown = true);

  // Deletes all objects which have requested for destruction
  void CollectDeadObjects();

  // =================================
  // RENDERING
  // =================================
public:
  // Registers the given Renderable to be rendered on future Render calls
  void RegisterLayerRenderer(std::shared_ptr<Renderable> component);

  // Sorts the render layer by the members render order
  void Sort(std::vector<std::weak_ptr<Renderable>> &components);

  // Gets all available cameras in this scene
  std::list<std::shared_ptr<Camera>> GetCameras();

private:
  // Registers a camera to the scene
  void RegisterCamera(std::shared_ptr<Camera> camera);

  // Stores it's cameras
  std::list<std::weak_ptr<Camera>> camerasWeak;

  // Structure that maps each render layer to the Renderables in it
  std::unordered_map<RenderLayer, std::vector<std::weak_ptr<Renderable>>> layerStructure;

  // =================================
  // UTILITY
  // =================================
public:
  // Gets this scene's shared pointer
  std::shared_ptr<GameScene> GetShared();

  // Supplies a valid unique id for a game object or a component
  int SupplyId();
};

#include "WorldComponent.h"
#include "Collider.h"

#endif