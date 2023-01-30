#include <algorithm>
#include "GameObject.h"
#include "Sound.h"
#include "Game.h"
#include <iostream>

using namespace std;

// Private constructor
GameObject::GameObject(string name, int gameSceneId, int id)
    : id(id >= 0 ? id : Game::GetInstance().SupplyId()), name(name), gameSceneId(gameSceneId)
{
}

// Default constructor
GameObject::GameObject(string name)
    : GameObject(name, Game::GetInstance().GetScene()->id)
{
  // Register to scene
  GetScene()->RegisterObject(this);
}

GameObject::~GameObject()
{
  cout << "In destructor of " << GetName() << endl;

  // Detect leaked components
  for (auto [componentId, component] : components)
    if (component.use_count() != 2)
      cout << "WARNING: Component " << typeid(*component).name() << " has " << component.use_count() - 2 << " leaked references" << endl;
}

void GameObject::Start()
{
  if (started)
    return;

  started = true;

  for (auto [componentId, component] : components)
    component->SafeStart();
}

void GameObject::Awake()
{
  if (awoke)
    return;

  awoke = true;

  for (auto [componentId, component] : components)
    component->Awake();
}

// Allows for registering to the scene's variables
void GameObject::RegisterToScene()
{
  auto currentScene = GetScene()->id;

  if (lastSceneRegisteredTo == currentScene)
    return;

  lastSceneRegisteredTo = currentScene;

  for (auto [componentId, component] : components)
    component->RegisterToSceneWithLayer();
}

void GameObject::Update(float deltaTime)
{
  // Apply timescale
  deltaTime *= GetTimeScale();

  // Update timers
  timer.Update(deltaTime);

  // Trigger delayed functions
  TriggerDelayedFunctions();

  if (enabled == false)
    return;

  for (auto [componentId, component] : components)
  {
    if (component->IsEnabled())
      component->Update(deltaTime);
  }
}

void GameObject::PhysicsUpdate(float deltaTime)
{
  if (enabled == false)
    return;

  deltaTime *= GetTimeScale();

  for (auto [componentId, component] : components)
  {
    if (component->IsEnabled())
      component->PhysicsUpdate(deltaTime);
  }
}

void GameObject::OnScenePause()
{
  for (auto [componentId, component] : components)
    component->OnScenePause();
}

void GameObject::OnSceneResume()
{
  for (auto [componentId, component] : components)
    component->OnSceneResume();
}

decltype(GameObject::components)::iterator GameObject::RemoveComponent(shared_ptr<Component> component)
{
  // Detect not present
  if (components.count(component->id) == 0)
    return components.end();

  // Wrap it up
  component->OnBeforeDestroy();

  // Remove it
  return components.erase(components.find(component->id));
}

auto GameObject::GetComponent(const Component *componentPointer) -> shared_ptr<Component>
{
  if (components.count(componentPointer->id) == 0)
    return nullptr;

  return components[componentPointer->id];
}

auto GameObject::RequireComponent(const Component *componentPointer) -> shared_ptr<Component>
{
  auto component = GetComponent(componentPointer);

  if (!component)
  {
    throw runtime_error(string("Required component was not found"));
  }

  return component;
}

// Absolute scale of the object
float GameObject::GetTimeScale() const
{
  if (IsRoot())
    return localTimeScale;

  return InternalGetParent()->GetTimeScale() * localTimeScale;
}

void GameObject::SetTimeScale(float newScale)
{
  Assert(newScale > 0, "Time must flow forward");

  if (IsRoot())
  {
    localTimeScale = newScale;
    return;
  }

  float parentTimeScale{InternalGetParent()->GetTimeScale()};

  Assert(parentTimeScale > 0, "Parent timeScale is invalid");

  localTimeScale = newScale / parentTimeScale;
}

void GameObject::InternalDestroy()
{
  // Wrap all components up
  for (
      auto componentIterator = components.begin();
      componentIterator != components.end();
      componentIterator = RemoveComponent(componentIterator->second))
    ;

  // Delete self from scene's list
  GetScene()->RemoveObject(id);
}

void GameObject::DontDestroyOnLoad(bool value)
{
  if (InternalGetParent()->IsRoot() == false)
  {
    cout << "WARNING: Tried to set non-root object to not destroy on load" << endl;
    return;
  }

  keepOnLoad = value;
}

shared_ptr<GameScene> GameObject::GetScene()
{
  auto currentScene = Game::GetInstance().GetScene();

  Assert(gameSceneId == currentScene->id, "Trying to access scene of object which is not in the current scene");

  return currentScene;
}

bool GameObject::operator==(const GameObject &other) const { return other.id == id; }
bool GameObject::operator!=(const GameObject &other) const { return !(*this == other); }

GameObject::operator string() const
{
  return "[" + GetName() + "::" + to_string(id) + "]";
}

ostream &operator<<(ostream &stream, const GameObject &object)
{
  stream << (string)object;
  return stream;
}

bool GameObject::IsEnabled() const
{
  if (enabled == false)
    return false;

  if (IsRoot())
    return true;

  return InternalGetParent()->IsEnabled();
}

void GameObject::TriggerDelayedFunctions()
{
  auto delayedFunctionEntryIterator = delayedFunctions.begin();
  while (delayedFunctionEntryIterator != delayedFunctions.end())
  {
    auto stringTokenId = to_string(delayedFunctionEntryIterator->first);

    // Check if it was disabled
    if (delayedFunctionEntryIterator->second.second == false)
      // Remove it
      delayedFunctionEntryIterator = delayedFunctions.erase(delayedFunctionEntryIterator);

    // Check if delay is up
    else if (timer.Get(stringTokenId) >= 0)
    {
      auto delayedFunction = delayedFunctionEntryIterator->second.first;

      // Scrap timer
      timer.Scrap(stringTokenId);

      // Trigger function
      delayedFunction();

      // Forget entry
      delayedFunctionEntryIterator = delayedFunctions.erase(delayedFunctionEntryIterator);
    }

    else
      delayedFunctionEntryIterator++;
  }
}

int GameObject::DelayFunction(function<void()> procedure, float seconds)
{
  // Get token id
  int tokenId = HashTwo(SDL_GetTicks(), RandomRange(0, 10000));

  // Store function
  delayedFunctions[tokenId] = {procedure, true};

  // Start timer
  timer.Reset(to_string(tokenId), -seconds);

  return tokenId;
}

void GameObject::CancelDelayedFunction(int tokenId)
{
  // Set it as not supposed to be called
  delayedFunctions[tokenId].second = false;
}

bool GameObject::DestroyRequested() const { return destroyRequested; }

void GameObject::RequestDestroy() { SetEnabled(false), destroyRequested = true; }

std::string GameObject::GetName() const { return name; }

void GameObject::SetEnabled(bool value) { enabled = value; }

bool GameObject::IsRoot() const { return id == 0; }
