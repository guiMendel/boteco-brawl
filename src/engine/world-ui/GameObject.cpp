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
    : GameObject(name, Game::GetInstance().GetScene()->id) {}

GameObject::~GameObject()
{
  MESSAGE << "In destructor of " << *this << endl;

  // Detect leaked components
  for (auto [componentId, component] : components)
    if (component.use_count() != 2)
      MESSAGE << "WARNING: Component " << typeid(*component).name() << " has " << component.use_count() - 2 << " leaked references" << endl;
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
  if (IsRoot())
    GetScene()->rootObject = nullptr;
  else
    GetScene()->RemoveObject(id);
}

void GameObject::DontDestroyOnLoad(bool value)
{
  if (InternalGetParent()->IsRoot() == false)
  {
    MESSAGE << "WARNING: Tried to set non-root object to not destroy on load" << endl;
    return;
  }

  keepOnLoad = value;
}

shared_ptr<GameScene> GameObject::GetScene() const
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

shared_ptr<GameObject> GameObject::InternalGetParent() const
{
  auto parent = InternalGetParentNoException();

  Assert(parent != nullptr, "Failed to retrieve parent of " + string(*this));

  return parent;
}
