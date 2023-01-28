#include <algorithm>
#include <math.h>
#include "GameState.h"
#include "Vector2.h"
#include "Sound.h"
#include "Camera.h"
#include "Resources.h"
#include "ObjectRecipes.h"
#include <iostream>

#define CASCADE_OBJECTS(method, param) CascadeDown(rootObject, [param](WorldObject &object) { object.method(param); });

using namespace std;

// Initialize root object
GameState::GameState()
    : physicsSystem(*this),
      particleSystem(*this),
      id(SupplyId()),
      inputManager(Game::GetInstance().GetInputManager()),
      rootObject(new WorldObject("Root", id, 0))
{
  // Set root object layer to default
  rootObject->physicsLayer = PhysicsLayer::Default;
}

GameState::~GameState()
{
  // Clear unused resources
  Resources::ClearAll();
}

void GameState::CascadeDown(shared_ptr<WorldObject> object, function<void(WorldObject &)> callback, bool topDown)
{
  // Execute on this object
  if (topDown)
    callback(*object);

  // Execute on it's children
  for (auto child : object->GetChildren())
    CascadeDown(child, callback, topDown);

  // Execute on this object (bottom up case)
  if (topDown == false)
    callback(*object);
}

void GameState::DeleteObjects()
{
  // Check for dead objects
  vector<weak_ptr<WorldObject>> deadObjects;

  // Collect them
  for (auto &objectPair : gameObjects)
  {
    // If is dead, collect
    if (objectPair.second->DestroyRequested())
    {
      deadObjects.emplace_back(objectPair.second);
    }

    // Not a good idea to delete them here directly, as it would invalidate this loop's iterator
  }

  // Erase them
  for (auto &weakObject : deadObjects)
    IF_LOCK(weakObject, deadObject)
    {
      deadObject->InternalDestroy();
    }
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

  // cout << gameObjects.size() << endl;

  OnUpdate(deltaTime);
}

void GameState::PhysicsUpdate(float deltaTime)
{
  // Physics update
  CASCADE_OBJECTS(PhysicsUpdate, deltaTime);

  // Resolve collisions
  // float startMs = SDL_GetTicks();
  physicsSystem.PhysicsUpdate(deltaTime);
  // cout << "Collisions took " << float(SDL_GetTicks()) - startMs << " ms" << endl;

  // Update particles
  particleSystem.PhysicsUpdate(deltaTime);
}

void GameState::Render()
{
  // Clear screen
  auto back = Camera::GetMain()->background;
  auto renderer = Game::GetInstance().GetRenderer();
  
  SDL_SetRenderDrawColor(renderer, back.red, back.green, back.blue, 255);
  SDL_RenderClear(renderer);

  // Foreach layer
  for (int layer{0}; layer != (int)RenderLayer::None; layer++)
  {
    // Get the layer's components
    auto &components = layerStructure[(RenderLayer)layer];

    // Sort them
    Sort(components);

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

void GameState::Sort(std::vector<std::weak_ptr<Component>> &components)
{
  // Component comparer
  // Must return true iff first parameter comes before second parameter
  auto comparer = [](weak_ptr<Component> comp1Weak, weak_ptr<Component> comp2Weak)
  {
    auto comp1 = comp1Weak.lock();
    auto comp2 = comp2Weak.lock();

    // Treat them as equal if any of them is null
    if (comp1 == nullptr || comp2 == nullptr)
      return false;

    return comp1->GetRenderOrder() < comp2->GetRenderOrder();
  };

  // Sort it
  sort(components.begin(), components.end(), comparer);
}

void GameState::Start()
{
  if (started)
    return;

  // Load any assets
  LoadAssets();

  // Create the initial objects
  InitializeObjects();

  awoke = true;

  // Wake objects
  CASCADE_OBJECTS(Awake, );

  started = true;

  // Register components to state
  CASCADE_OBJECTS(RegisterToState, );

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

shared_ptr<WorldObject> GameState::RegisterObject(shared_ptr<WorldObject> worldObject)
{
  gameObjects[worldObject->id] = worldObject;
  worldObject->gameStateId = id;

  if (awoke)
    worldObject->Awake();

  if (started)
  {
    // Register this object's hierarchy to this new state
    worldObject->RegisterToState();
    worldObject->Start();
  }

  return worldObject;
}

shared_ptr<WorldObject> GameState::RegisterObject(WorldObject *worldObject) { return RegisterObject(shared_ptr<WorldObject>(worldObject)); }

shared_ptr<WorldObject> GameState::GetPointer(const WorldObject *targetObject)
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
}

shared_ptr<WorldObject> GameState::GetObject(int id)
{
  if (gameObjects.count(id) == 0)
    return nullptr;

  return gameObjects.at(id);
}

shared_ptr<WorldObject> GameState::RequireObject(int id)
{
  auto object = GetObject(id);

  Assert(object != nullptr, "ERROR: Required an object with id " + to_string(id) + ", but couldn't find one");

  return object;
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

int GameState::SupplyId() { return Game::GetInstance().SupplyId(); }

vector<shared_ptr<WorldObject>> GameState::GetObjectsToCarryOn()
{
  vector<shared_ptr<WorldObject>> savedObjects;

  for (auto firstObject : GetRootObject()->GetChildren())
    if (firstObject->keepOnLoad)
      savedObjects.push_back(firstObject);

  return savedObjects;
}

shared_ptr<GameState> GameState::GetShared()
{
  auto currentState = Game::GetInstance().GetState();

  Assert(id == currentState->id, "Tried to get shared pointer of inactive game state");

  return currentState;
}

shared_ptr<WorldObject> GameState::FindObject(string name)
{
  for (auto [objectId, object] : gameObjects)
    if (object->GetName() == name)
      return object;

  return nullptr;
}
