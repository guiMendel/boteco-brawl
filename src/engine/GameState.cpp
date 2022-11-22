#include <algorithm>
#include <math.h>
#include "GameState.h"
#include "Vector2.h"
#include "Sound.h"
#include "Camera.h"
#include "Resources.h"
#include "Recipes.h"
#include <iostream>

#define CASCADE_OBJECTS(method, param) CascadeDown(rootObject, [param](GameObject &object) { object.method(param); });

using namespace std;

// Initialize root object
GameState::GameState()
    : physicsSystem(*this), inputManager(Game::GetInstance().GetInputManager()), rootObject(new GameObject("Root", *this))
{
}

GameState::~GameState()
{
  // Clear unused resources
  Resources::ClearAll();
}

void GameState::CascadeDown(shared_ptr<GameObject> object, function<void(GameObject &)> callback, bool topDown)
{
  // Execute on this object
  if (topDown)
    callback(*object);

  // Update it's children
  for (auto child : object->GetChildren())
    CascadeDown(child, callback, topDown);

  // Execute on this object (bottom up case)
  if (topDown == false)
    callback(*object);
}

void GameState::DeleteObjects()
{
  // Check for dead objects
  vector<shared_ptr<GameObject>> deadObjects;

  // Collect them
  for (auto &objectPair : gameObjects)
  {
    // If is dead, collect
    if (objectPair.second->DestroyRequested())
    {
      deadObjects.push_back(objectPair.second);
    }

    // Not a good idea to delete them here directly, as it would invalidate this loop's iterator
  }

  // Erase them
  for (auto &deadObject : deadObjects)
    deadObject->InternalDestroy();
}

void GameState::Update(float deltaTime)
{
  // Quit if necessary
  if (inputManager.QuitRequested())
  {
    quitRequested = true;
  }

  // Update game objects
  CASCADE_OBJECTS(Update, deltaTime);

  // Delete dead ones
  DeleteObjects();
}

void GameState::PhysicsUpdate(float deltaTime)
{
  // Physics update
  CASCADE_OBJECTS(PhysicsUpdate, deltaTime);

  // Resolve collisions
  physicsSystem.PhysicsUpdate(deltaTime);
}

void GameState::Render()
{
  // Foreach layer
  for (int layer{0}; layer != (int)RenderLayer::None; layer++)
  {
    // Get the layer's components
    auto &components = layerStructure[(RenderLayer)layer];

    // For each component in this layer
    auto componentIterator{components.begin()};

    while (componentIterator != components.end())
    {
      // Lock the component
      if (auto component = componentIterator->lock())
      {
        // Render it
        if (component->IsEnabled())
          component->Render();

        // Advance
        componentIterator++;
      }
      // If lock fails, it was erased, so remove it
      else
      {
        componentIterator = components.erase(componentIterator);
      }
    }
  }
}

void GameState::Start()
{

  if (started)
    return;

  // Load any assets
  LoadAssets();

  // Create the main camera
  auto mainCamera = CreateObject("MainCamera", Recipes::Camera())->GetComponent<Camera>();

  // Register it before going on
  RegisterCamera(mainCamera);

  // Create the initial objects
  InitializeObjects();

  started = true;

  // Start objects
  CASCADE_OBJECTS(Start, );

  // Start physics system
  physicsSystem.Start();
}

void GameState::Pause()
{
  // Communicate to objects
  CASCADE_OBJECTS(OnStatePause, );
}

void GameState::Resume()
{
  // Communicate to objects
  CASCADE_OBJECTS(OnStateResume, );
}

void GameState::RemoveObject(int id)
{
  gameObjects.erase(id);
}

shared_ptr<GameObject> GameState::RegisterObject(GameObject *gameObject)
{
  gameObjects[gameObject->id] = shared_ptr<GameObject>(gameObject);
  return gameObjects[gameObject->id];
}

shared_ptr<GameObject> GameState::GetPointer(const GameObject *targetObject)
{
  // Find this pointer in the list
  auto foundObjectIterator = find_if(
      gameObjects.begin(), gameObjects.end(),
      [targetObject](const auto objectPair)
      { return objectPair.second.get() == targetObject; });

  // Catch nonexistent
  if (foundObjectIterator == gameObjects.end())
  {
    // Return empty pointer
    return nullptr;
  }

  return foundObjectIterator->second;
}

void GameState::RegisterLayerRenderer(shared_ptr<Component> component)
{
  // Simply ignore invalid requests
  if (!component)
    return;

  // Get it's layer
  auto &layer = layerStructure[component->GetRenderLayer()];

  // Add it's entry
  layer.emplace_back(component);

  // Sort it
  sort(layer.begin(), layer.end(), [](weak_ptr<Component> comp1Weak, weak_ptr<Component> comp2Weak)
       { 
        auto comp1 = comp1Weak.lock();
        auto comp2 = comp2Weak.lock();

        // Leave as is if any of them has been erased
        if (comp1 == nullptr || comp2 == nullptr) return true;
        
        return comp1->GetRenderOrder() < comp2->GetRenderOrder(); });
}

shared_ptr<GameObject> GameState::GetObject(int id)
{
  try
  {
    return gameObjects.at(id);
  }
  catch (out_of_range &)
  {
    return nullptr;
  }
}

void GameState::RegisterCamera(shared_ptr<Camera> camera)
{
  auto cameras = GetCameras();

  if (find_if(cameras.begin(), cameras.end(), [camera](shared_ptr<Camera> &otherCamera)
              { return camera->id == otherCamera->id; }) == cameras.end())
    camerasWeak.emplace_back(camera);
}

list<shared_ptr<Camera>> GameState::GetCameras()
{
  return ParseWeakIntoShared(camerasWeak);
}
