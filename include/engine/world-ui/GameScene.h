#ifndef __GAME_SCENE__
#define __GAME_SCENE__

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <list>
#include <SDL.h>
#include "WorldObject.h"
#include "SpriteRenderer.h"
#include "Music.h"
#include "InputManager.h"
#include "Vector2.h"
#include "PhysicsSystem.h"
#include "ParticleSystem.h"

class Component;
class Collider;
class Camera;
class Game;

// Abstract class that defines a scene of the game
class GameScene
{
  friend class Collider;
  friend class Camera;
  friend class WorldObject;
  friend class Game;

public:
  GameScene();

  virtual ~GameScene();

  // Whether the game should exit
  bool QuitRequested() { return quitRequested; }

  // Whether to remove this scene from the queue
  bool PopRequested() { return popRequested; }

  void Update(float deltaTime);
  virtual void OnUpdate(float) {}
  virtual void PhysicsUpdate(float deltaTime);

  virtual void Render();

  virtual void Start();

  virtual void Pause();

  virtual void Resume();

  // Removes an object from the object list
  void RemoveObject(int id);

  void RemoveObject(std::shared_ptr<WorldObject> worldObject) { RemoveObject(worldObject->id); }

  std::shared_ptr<WorldObject> RegisterObject(WorldObject *worldObject);
  std::shared_ptr<WorldObject> RegisterObject(std::shared_ptr<WorldObject> worldObject);

  // Creates a new world object
  template <typename... Args>
  std::shared_ptr<WorldObject> CreateObject(
      std::string name, std::function<void(std::shared_ptr<WorldObject>)> recipe = nullptr, Args &&...args)
  {
    // Create the object, which automatically registers it's pointer to the scene's list
    int objectId = (new WorldObject(name, std::forward<Args>(args)...))->id;
    auto object = gameObjects[objectId];

    // Initialize it
    if (recipe)
      recipe(object);

    return object;
  }

  std::shared_ptr<WorldObject> GetPointer(const WorldObject *worldObject);

  std::shared_ptr<WorldObject> GetObject(int id);

  // Throws if this object doesn't exist
  std::shared_ptr<WorldObject> RequireObject(int id);

  std::shared_ptr<GameScene> GetShared();

  template <class T>
  auto FindObjectOfType() -> std::shared_ptr<T>
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

  template <class T>
  auto RequireObjectOfType() -> std::shared_ptr<T>
  {
    auto object = FindObjectOfType<T>();

    Helper::Assert(object != nullptr, "Required object type was not present in game scene");

    return object;
  }

  std::shared_ptr<WorldObject> FindObject(std::string name);

  // Initializes the scene's objects
  virtual void InitializeObjects() = 0;

  // Preloads all the assets so that they are ready when required
  virtual void LoadAssets() {}

  // Supplies a valid unique id for a world object or a component
  int SupplyId();

  void RegisterLayerRenderer(std::shared_ptr<Component> component);

  // Sorts the layer by the components render order
  void Sort(std::vector<std::weak_ptr<Component>> &components);

  std::list<std::shared_ptr<Camera>> GetCameras();

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

  // Array with all of the scene's objects
  std::unordered_map<int, std::shared_ptr<WorldObject>> gameObjects;

  // Root object reference
  std::shared_ptr<WorldObject> rootObject;

  // Indicates that the scene mus tbe removed from queue
  bool popRequested{false};

  // Indicates that the game must exit
  bool quitRequested{false};

private:
  std::shared_ptr<WorldObject> GetRootObject() { return rootObject; }

  // Gets all objects which must be carried on to the next frame
  std::vector<std::shared_ptr<WorldObject>> GetObjectsToCarryOn();

  void RegisterCamera(std::shared_ptr<Camera> camera);

  // Executes this function for each object, cascading down the hierarchy
  void CascadeDown(std::shared_ptr<WorldObject> object, std::function<void(WorldObject &)> callback, bool topDown = true);
  void DeleteObjects();

  // Whether the scene has executed the start method
  bool started{false};
  bool awoke{false};

  // Stores it's cameras
  std::list<std::weak_ptr<Camera>> camerasWeak;

  // Structure that maps each render layer to the components set to render in it
  std::unordered_map<RenderLayer, std::vector<std::weak_ptr<Component>>>
      layerStructure;
};

#include "Component.h"
#include "Collider.h"

#endif