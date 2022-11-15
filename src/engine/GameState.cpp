#include <algorithm>
#include <math.h>
#include "GameState.h"
#include "Vector2.h"
#include "Sound.h"
#include "TileMap.h"
#include "TileSet.h"
#include "Camera.h"
#include "Resources.h"
#include "SatCollision.h"
#include <iostream>

#define CASCADE_OBJECTS(method, param) CascadeDown(rootObject, [param](GameObject &object) { object.method(param); });

using namespace std;

// Whether the two collider lists have some pair of colliders which are colliding
bool CheckForCollision(vector<shared_ptr<Collider>> colliders1, vector<shared_ptr<Collider>> colliders2)
{
  for (auto collider1 : colliders1)
  {
    for (auto collider2 : colliders2)
    {
      if (SatCollision::IsColliding(
              collider1->GetBox(), collider2->GetBox(),
              collider1->gameObject.GetRotation(), collider2->gameObject.GetRotation(),
              pow(collider1->GetMaxVertexDistance() + collider2->GetMaxVertexDistance(), 2)))
      {
        return true;
      }
    }
  }

  return false;
}

// Initialize root object
GameState::GameState() : inputManager(InputManager::GetInstance()), rootObject(new GameObject("Root", *this))
{
}

GameState::~GameState()
{
  // Clear unused resources
  Resources::ClearAll();

  // Reset camera
  Camera::GetInstance().Reset();
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

void GameState::DetectCollisions()
{
  // Get validated colliders
  auto validatedColliderStructure = ValidateColliders();

  // For each object
  auto objectEntryIterator = validatedColliderStructure.begin();
  while (objectEntryIterator != validatedColliderStructure.end())
  {
    // Test, for each OTHER object in the list (excluding the ones before this one)
    decltype(objectEntryIterator) otherObjectEntryIterator{objectEntryIterator};
    otherObjectEntryIterator++;
    while (otherObjectEntryIterator != validatedColliderStructure.end())
    {
      // Check if they are colliding
      if (CheckForCollision(objectEntryIterator->second, otherObjectEntryIterator->second))
      {
        auto &object1 = *gameObjects[objectEntryIterator->first];
        auto &object2 = *gameObjects[otherObjectEntryIterator->first];
        object1.OnCollision(object2);
        object2.OnCollision(object1);
      }

      otherObjectEntryIterator++;
    }

    objectEntryIterator++;
  }
}

void GameState::Update(float deltaTime)
{
  // Quit if necessary
  if (inputManager.QuitRequested())
  {
    quitRequested = true;
  }

  // Update camera
  Camera::GetInstance().Update(deltaTime);

  // Update game objects
  CASCADE_OBJECTS(Update, deltaTime);

  // Delete dead ones
  DeleteObjects();

  // Inform them of any collisions
  DetectCollisions();
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

  // Create the initial objects
  InitializeObjects();

  started = true;

  // Start objects
  CASCADE_OBJECTS(Start, );
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

void GameState::RegisterCollider(shared_ptr<Collider> collider)
{
  if (!collider)
    return;

  colliderStructure[collider->gameObject.id].emplace_back(collider);
}

unordered_map<int, vector<shared_ptr<Collider>>> GameState::ValidateColliders()
{
  unordered_map<int, vector<shared_ptr<Collider>>> verifiedCollidersStructure;

  // For each object entry
  auto objectEntryIterator = colliderStructure.begin();
  while (objectEntryIterator != colliderStructure.end())
  {
    auto &objectColliders = objectEntryIterator->second;

    // For each of it's colliders
    auto colliderIterator = objectColliders.begin();
    while (colliderIterator != objectColliders.end())
    {
      // Remove it if it's expired
      if (colliderIterator->expired())
      {
        colliderIterator = objectColliders.erase(colliderIterator);
        continue;
      }

      // Otherwise lock it and add it
      verifiedCollidersStructure[objectEntryIterator->first]
          .push_back(colliderIterator->lock());

      // Advance
      colliderIterator++;
    }

    // If it's empty, remove it from the map
    if (objectColliders.empty())
    {
      objectEntryIterator = colliderStructure.erase(objectEntryIterator);
      continue;
    }

    objectEntryIterator++;
  }

  return verifiedCollidersStructure;
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
