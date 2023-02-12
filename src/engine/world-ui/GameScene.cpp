#include <algorithm>
#include <math.h>
#include "GameScene.h"
#include "Vector2.h"
#include "Sound.h"
#include "Camera.h"
#include "Resources.h"
#include "Parent.h"
#include <iostream>

#define CASCADE_OBJECTS(method, param) CascadeDown([param](GameObject &object) { object.method(param); });

using namespace std;

// Initialize root object
GameScene::GameScene()
    : physicsSystem(*this),
      particleSystem(*this),
      id(SupplyId()),
      inputManager(Game::GetInstance().GetInputManager()),
      rootObject(new WorldObject("Root", id, 0))
{
  // Set root object layer to default
  rootObject->physicsLayer = PhysicsLayer::Default;
}

GameScene::~GameScene()
{
  Assert(gameObjects.size() == 0, "Failed to destroy all scene's objects before reaching it's destructor");
}

void GameScene::CascadeDown(function<void(GameObject &)> callback, bool topDown)
{
  // Cascade to root
  GetRootObject()->CascadeDown(callback, topDown);
}

void GameScene::CollectDeadObjects()
{
  // Check for dead objects
  vector<weak_ptr<GameObject>> deadObjects;

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

void GameScene::Update(float deltaTime)
{
  // Quit if necessary
  if (inputManager.QuitRequested())
  {
    quitRequested = true;
  }

  // Update world objects
  CASCADE_OBJECTS(Update, deltaTime);

  // Delete dead ones
  CollectDeadObjects();

  // cout << gameObjects.size() << endl;

  OnUpdate(deltaTime);
}

void GameScene::PhysicsUpdate(float deltaTime)
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

void GameScene::Render()
{
  // Clear screen
  auto back = Camera::GetMain()->background;
  auto renderer = Game::GetInstance().GetRenderer();

  SDL_SetRenderDrawColor(renderer, back.red, back.green, back.blue, 255);
  SDL_RenderClear(renderer);

  // Foreach layer
  for (int layer{0}; layer != (int)RenderLayer::None; layer++)
  {
    // Get the layer's Renderables
    auto &renderables = layerStructure[(RenderLayer)layer];

    // Sort them
    Sort(renderables);

    // For each Renderable in this layer
    auto renderableIterator{renderables.begin()};

    while (renderableIterator != renderables.end())
    {
      // Lock the Renderable
      if (auto renderable = renderableIterator->lock())
      {
        // Render it
        if (renderable->ShouldRender())
          renderable->Render();

        // Advance
        renderableIterator++;
      }
      // If lock fails, it was erased, so remove it
      else
      {
        renderableIterator = renderables.erase(renderableIterator);
      }
    }
  }
}

void GameScene::Sort(vector<weak_ptr<Renderable>> &renderables)
{
  // Renderable comparer
  // Must return true iff first parameter comes before second parameter
  auto comparer = [](weak_ptr<Renderable> comp1Weak, weak_ptr<Renderable> comp2Weak)
  {
    auto comp1 = comp1Weak.lock();
    auto comp2 = comp2Weak.lock();

    // Treat them as equal if any of them is null
    if (comp1 == nullptr || comp2 == nullptr)
      return false;

    return comp1->GetRenderOrder() < comp2->GetRenderOrder();
  };

  // Sort it, preserving layer register order for equivalent members
  stable_sort(renderables.begin(), renderables.end(), comparer);
}

void GameScene::Start()
{
  if (started)
    return;

  // Create the initial objects
  InitializeObjects();

  awoke = true;

  // Wake objects
  CASCADE_OBJECTS(Awake, );

  started = true;

  // Register renderables to scene
  CASCADE_OBJECTS(RegisterToScene, );

  // Start objects
  CASCADE_OBJECTS(Start, );

  // Start physics system
  physicsSystem.Start();
}

void GameScene::Pause()
{
  // Communicate to objects
  CASCADE_OBJECTS(OnScenePause, );
}

void GameScene::Resume()
{
  // Communicate to objects
  CASCADE_OBJECTS(OnSceneResume, );
}

void GameScene::RemoveObject(int id)
{
  Assert(id != 0, "Cannot destroy root object with this method");

  gameObjects.erase(id);
}

shared_ptr<GameObject> GameScene::RegisterObject(shared_ptr<GameObject> gameObject)
{
  gameObjects[gameObject->id] = gameObject;
  gameObject->gameSceneId = id;

  // Ensure it's parent has a reference to it
  gameObject->InternalSetParent(gameObject->InternalGetParent());

  if (awoke)
    gameObject->Awake();

  if (started)
  {
    // Register this object's hierarchy to this new scene
    gameObject->RegisterToScene();
    gameObject->Start();
  }

  return gameObject;
}

shared_ptr<GameObject> GameScene::RegisterObject(GameObject *gameObject) { return RegisterObject(shared_ptr<GameObject>(gameObject)); }

void GameScene::RegisterLayerRenderer(shared_ptr<Renderable> renderable)
{
  // Simply ignore invalid requests
  if (!renderable)
    return;

  // Get it's layer
  auto &layer = layerStructure[renderable->GetRenderLayer()];

  // Add it's entry
  layer.emplace_back(renderable);
}

shared_ptr<GameObject> GameScene::GetGameObject(int id)
{
  if (gameObjects.count(id) == 0)
    return nullptr;

  return gameObjects.at(id);
}

shared_ptr<GameObject> GameScene::RequireGameObject(int id)
{
  auto object = GetGameObject(id);

  Assert(object != nullptr, "ERROR: Required an object with id " + to_string(id) + ", but couldn't find one");

  return object;
}

void GameScene::RegisterCamera(shared_ptr<Camera> camera)
{
  auto cameras = GetCameras();

  if (find_if(cameras.begin(), cameras.end(), [camera](shared_ptr<Camera> &otherCamera)
              { return camera->id == otherCamera->id; }) == cameras.end())
    camerasWeak.emplace_back(camera);
}

list<shared_ptr<Camera>> GameScene::GetCameras()
{
  return ParseWeakIntoShared(camerasWeak);
}

int GameScene::SupplyId() { return Game::GetInstance().SupplyId(); }

vector<shared_ptr<GameObject>> GameScene::ExtractObjectsToCarryOn()
{
  vector<shared_ptr<GameObject>> savedObjects;

  // Direct root children are the only ones that can be carried on
  for (auto child : GetRootObject()->GetChildren())
    if (child->keepOnLoad)
    {
      // Unlink it from root
      child->UnlinkParent();

      // Remove it and it's children from this scene and keep their pointers
      auto extractObject = [this, &savedObjects](GameObject &object)
      {
        // Keep it's pointer to return it
        savedObjects.push_back(RequireGameObject(object.id));

        // Remove the object from this scene
        RemoveObject(object.id);
      };

      child->CascadeDown(extractObject);
    }

  return savedObjects;
}

shared_ptr<GameScene> GameScene::GetShared()
{
  auto currentScene = Game::GetInstance().GetScene();

  Assert(id == currentScene->id, "Tried to get shared pointer of inactive game scene");

  return currentScene;
}

shared_ptr<GameObject> GameScene::GetGameObject(string name)
{
  for (auto [objectId, object] : gameObjects)
    if (object->GetName() == name)
      return object;

  return nullptr;
}

void GameScene::RemoveObject(std::shared_ptr<GameObject> gameObject) { RemoveObject(gameObject->id); }

std::shared_ptr<WorldObject> GameScene::GetWorldObject(int id)
{
  auto gameObject = GetGameObject(id);

  if (gameObject == nullptr)
    return nullptr;

  return RequirePointerCast<WorldObject>(gameObject);
}

std::shared_ptr<WorldObject> GameScene::GetWorldObject(std::string name)
{
  auto gameObject = GetGameObject(name);

  if (gameObject == nullptr)
    return nullptr;

  return RequirePointerCast<WorldObject>(gameObject);
}

std::shared_ptr<GameObject> GameScene::RequireGameObject(std::string name)
{
  auto gameObject = GetGameObject(name);

  Assert(gameObject != nullptr, "ERROR: Required an object with name " + name + ", but couldn't find one");

  return gameObject;
}

std::shared_ptr<WorldObject> GameScene::RequireWorldObject(int id)
{
  return RequirePointerCast<WorldObject>(RequireGameObject(id));
}

std::shared_ptr<WorldObject> GameScene::RequireWorldObject(std::string name)
{
  return RequirePointerCast<WorldObject>(RequireGameObject(name));
}

std::shared_ptr<WorldObject> GameScene::GetRootObject() { return rootObject; }

void GameScene::Destroy()
{
  // Destroy root object
  rootObject->InternalDestroy();

  // Ensure they were all destroyed
  Assert(gameObjects.size() == 0, "Failed to destroy all objects before destroying scene");

  // Clear unused resources
  Resources::ClearAll();
}
