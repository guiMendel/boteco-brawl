#include <algorithm>
#include "WorldObject.h"
#include "Sound.h"
#include "Game.h"
#include <iostream>

using namespace std;

const float WorldObject::objectCollectionRange{50};

// Private constructor
WorldObject::WorldObject(string name, int gameSceneId, int id)
    : id(id >= 0 ? id : Game::GetInstance().SupplyId()), name(name), gameSceneId(gameSceneId)
{
}

// With dimensions
WorldObject::WorldObject(string name, Vector2 coordinates, double rotation, shared_ptr<WorldObject> parent)
    : WorldObject(name, Game::GetInstance().GetScene()->id)
{
  // Add gameScene reference
  auto gameScene = GetScene();
  auto shared = gameScene->RegisterObject(this);

  // Only add a parent if not the root object
  if (IsRoot() == false)
  {
    // If no parent, add root as parent
    if (parent == nullptr)
      parent = gameScene->GetRootObject();

    // Add reference to parent
    this->weakParent = parent;

    // Give parent a reference to self
    parent->children[id] = weak_ptr(shared);
  }

  // Inherit parent's layer (don't use SetPhysicsLayer here, as we don't want to set inheritedPhysicsLayer to false)
  physicsLayer = parent->physicsLayer;

  SetPosition(coordinates);
  SetRotation(rotation);
}

WorldObject::~WorldObject()
{
  cout << "In destructor of " << GetName() << endl;

  // Detect leaked components
  for (auto [componentId, component] : components)
    if (component.use_count() != 2)
      cout << "WARNING: Component " << typeid(*component).name() << " has " << component.use_count() - 2 << " leaked references" << endl;
}

void WorldObject::Start()
{
  if (started)
    return;

  started = true;

  for (auto [componentId, component] : components)
    component->SafeStart();
}

void WorldObject::Awake()
{
  if (awoke)
    return;

  awoke = true;

  for (auto [componentId, component] : components)
    component->Awake();
}

// Allows for registering to the scene's variables
void WorldObject::RegisterToScene()
{
  auto currentScene = GetScene()->id;

  if (lastSceneRegisteredTo == currentScene)
    return;

  lastSceneRegisteredTo = currentScene;

  for (auto [componentId, component] : components)
    component->RegisterToSceneWithLayer();
}

void WorldObject::Update(float deltaTime)
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

  // Check range collection
  auto absolutePosition = GetPosition().GetAbsolute();

  if (absolutePosition.x > objectCollectionRange || absolutePosition.y > objectCollectionRange)
  {
    cout << "Collecting " << *this << " for exceeding collection range" << endl;

    RequestDestroy();
  }
}

void WorldObject::PhysicsUpdate(float deltaTime)
{
  if (enabled == false)
    return;

  deltaTime *= GetTimeScale();

  // Check for collision & trigger exit
  DetectCollisionExits();

  for (auto [componentId, component] : components)
  {
    if (component->IsEnabled())
      component->PhysicsUpdate(deltaTime);
  }
}

void WorldObject::DetectCollisionExits()
{
  // For each of last frame's collisions
  for (auto [collisionHash, collision] : lastFrameCollisions)
  {
    // Check colliders
    // If it hasn't happened this frame
    if (collision.weakOther.expired() == false &&
        collision.weakSource.expired() == false &&
        frameCollisions.count(collisionHash) == 0)
      // Raise exit
      OnCollisionExit(collision);
  }

  // For each of last frame's triggers
  for (auto [triggerHash, triggerData] : lastFrameTriggers)
  {
    // Check colliders
    // If it hasn't happened this frame
    if (triggerData.weakOther.expired() == false &&
        triggerData.weakSource.expired() == false &&
        frameTriggers.count(triggerHash) == 0)
      // Raise exit
      OnTriggerCollisionExit(triggerData);
  }

  // Update registers
  lastFrameCollisions = frameCollisions;
  lastFrameTriggers = frameTriggers;
  frameCollisions.clear();
  frameTriggers.clear();
}

void WorldObject::OnScenePause()
{
  for (auto [componentId, component] : components)
    component->OnScenePause();
}

void WorldObject::OnSceneResume()
{
  for (auto [componentId, component] : components)
    component->OnSceneResume();
}

decltype(WorldObject::components)::iterator WorldObject::RemoveComponent(shared_ptr<Component> component)
{
  // Detect not present
  if (components.count(component->id) == 0)
    return components.end();

  HandleColliderDestruction(component);

  // Wrap it up
  component->OnBeforeDestroy();

  // Remove it
  return components.erase(components.find(component->id));
}

void WorldObject::HandleColliderDestruction(shared_ptr<Component> component)
{
  // Get as a collider
  auto colliderToDie = dynamic_pointer_cast<Collider>(component);

  // Check if it is indeed a collider
  if (colliderToDie == nullptr)
    return;

  // Trigger collisions to exit
  unordered_map<size_t, TriggerCollisionData> exitTriggers;

  // For each of this frame's triggers
  for (auto [triggerHash, triggerData] : frameTriggers)
  {
    // Raise if the source collider is this one
    IF_LOCK(triggerData.weakSource, source)
    IF_LOCK(triggerData.weakOther, other)
    if (source->id == colliderToDie->id)
      exitTriggers[triggerHash] = triggerData;
  }

  // For each of last frame's triggers
  for (auto [triggerHash, triggerData] : lastFrameTriggers)
  {
    // Raise if the source collider is this one
    IF_LOCK(triggerData.weakSource, source)
    IF_LOCK(triggerData.weakOther, other)
    if (source->id == colliderToDie->id)
      exitTriggers[triggerHash] = triggerData;
  }

  // Exit all collected triggers
  for (auto [triggerHash, triggerData] : exitTriggers)
  {
    LOCK(triggerData.weakOther, other);

    OnTriggerCollisionExit(triggerData);

    auto otherData = triggerData;
    otherData.weakOther = triggerData.weakSource;
    otherData.weakSource = triggerData.weakOther;

    other->OnTriggerCollisionExit(otherData);
  }

  // Collisions to exit
  unordered_map<size_t, Collision::Data> exitCollisions;

  // For each of this frame's collisions
  for (auto [collisionHash, collisionData] : frameCollisions)
  {
    // Raise if the source collider is this one
    IF_LOCK(collisionData.weakSource, source)
    IF_LOCK(collisionData.weakOther, other)
    if (source->id == colliderToDie->id)
      exitCollisions[collisionHash] = collisionData;
  }

  // For each of last frame's collisions
  for (auto [collisionHash, collisionData] : lastFrameCollisions)
  {
    // Raise if the source collider is this one
    IF_LOCK(collisionData.weakSource, source)
    IF_LOCK(collisionData.weakOther, other)
    if (source->id == colliderToDie->id)
      exitCollisions[collisionHash] = collisionData;
  }

  // Exit all collected collisions
  for (auto [collisionHash, collisionData] : exitCollisions)
  {
    LOCK(collisionData.weakOther, other);

    OnCollisionExit(collisionData);

    auto otherData = collisionData;
    otherData.weakOther = collisionData.weakSource;
    otherData.weakSource = collisionData.weakOther;

    other->OnCollisionExit(otherData);
  }
}

shared_ptr<WorldObject> WorldObject::GetShared()
{
  return GetScene()->GetObject(id);
}

auto WorldObject::GetComponent(const Component *componentPointer) -> shared_ptr<Component>
{
  if (components.count(componentPointer->id) == 0)
    return nullptr;

  return components[componentPointer->id];
}

auto WorldObject::RequireComponent(const Component *componentPointer) -> shared_ptr<Component>
{
  auto component = GetComponent(componentPointer);

  if (!component)
  {
    throw runtime_error(string("Required component was not found"));
  }

  return component;
}

shared_ptr<WorldObject> WorldObject::InternalGetParent() const
{
  // Ensure not root
  Assert(IsRoot() == false, "Getting parent is forbidden on root object");

  if (IsRoot())
    return nullptr;

  // Ensure the parent is there
  Assert(weakParent.expired() == false, "WorldObject " + name + " unexpectedly failed to retrieve parent object");

  return weakParent.lock();
}

shared_ptr<WorldObject> WorldObject::GetParent() const
{
  auto parent = InternalGetParent();

  return parent->id == 0 ? nullptr : parent;
}

auto WorldObject::UnlinkParent() -> unordered_map<int, weak_ptr<WorldObject>>::iterator
{
  Assert(IsRoot() == false, "Root has no parent to unlink");

  // Get own iterator
  auto iterator = InternalGetParent()->children.find(id);

  // Remove it
  iterator = InternalGetParent()->children.erase(iterator);
  weakParent.reset();

  return iterator;
}

void WorldObject::SetParent(shared_ptr<WorldObject> newParent)
{
  Assert(IsRoot() == false, "SetParent is forbidden on root object");

  // Delete current parent
  UnlinkParent();

  // Set new parent
  weakParent = newParent;

  // Inherit this new parent's layer if necessary
  if (inheritedPhysicsLayer)
    physicsLayer = newParent->physicsLayer;
}

// Where this object exists in game space, in absolute coordinates
Vector2 WorldObject::GetPosition()
{
  if (IsRoot())
    return localPosition;

  // if (GetName() == "DashParticles")
  //   cout << localPosition << endl;

  return InternalGetParent()->GetPosition() + localPosition;
}
void WorldObject::SetPosition(const Vector2 newPosition)
{
  if (IsRoot())
    localPosition = newPosition;
  localPosition = newPosition - InternalGetParent()->GetPosition();
}

void WorldObject::Translate(const Vector2 translation)
{
  SetPosition(GetPosition() + translation);
}

// Absolute scale of the object
float WorldObject::GetTimeScale() const
{
  if (IsRoot())
    return localTimeScale;

  return InternalGetParent()->GetTimeScale() * localTimeScale;
}

void WorldObject::SetTimeScale(float newScale)
{
  Assert(newScale > 0, "Time must flow forward");

  if (IsRoot())
    localTimeScale = newScale;

  float parentTimeScale{InternalGetParent()->GetTimeScale()};

  Assert(parentTimeScale > 0, "Parent timeScale is invalid");

  localTimeScale = newScale / parentTimeScale;
}

// Absolute scale of the object
Vector2 WorldObject::GetScale()
{
  if (IsRoot())
    return localScale;
  return InternalGetParent()->GetScale() * localScale;
}
void WorldObject::SetScale(const Vector2 newScale)
{
  if (IsRoot())
    localScale = newScale;

  Vector2 parentScale{InternalGetParent()->GetScale()};

  Assert(parentScale.x > 0 && parentScale.y > 0, "Parent scale had a 0 is invalid");

  localScale = newScale / parentScale;
}

// Absolute rotation in radians
double WorldObject::GetRotation()
{
  if (IsRoot())
    return localRotation;
  return InternalGetParent()->GetRotation() + localRotation;
}
void WorldObject::SetRotation(const double newRotation)
{
  if (IsRoot())
    localRotation = newRotation;
  localRotation = newRotation - InternalGetParent()->GetRotation();
}

shared_ptr<WorldObject> WorldObject::CreateChild(string name)
{
  return CreateChild(name, Vector2::Zero(), 0);
}

shared_ptr<WorldObject> WorldObject::CreateChild(string name, Vector2 offset)
{
  return CreateChild(name, offset, 0);
}

shared_ptr<WorldObject> WorldObject::CreateChild(string name, Vector2 offset, float offsetRotation)
{
  auto childId = (new WorldObject(name, GetPosition() + offset, GetRotation() + offsetRotation, GetShared()))->id;
  return GetScene()->GetObject(childId);
}

vector<shared_ptr<WorldObject>> WorldObject::GetChildren()
{
  vector<shared_ptr<WorldObject>> verifiedChildren;

  // For each child entry
  auto childEntryIterator = children.begin();
  while (childEntryIterator != children.end())
  {
    // If it's expired
    if (childEntryIterator->second.expired())
    {
      // Remove it
      childEntryIterator = children.erase(childEntryIterator);

      continue;
    }

    // Otherwise lock it and add it
    verifiedChildren.push_back(childEntryIterator->second.lock());

    // Advance
    childEntryIterator++;
  }

  return verifiedChildren;
}

shared_ptr<WorldObject> WorldObject::RequireChild(int id)
{
  auto child = GetChild(id);

  // cout << "Children:" << endl;
  // for (auto child : children)
  //   cout << *child.second.lock() << endl;

  Assert(child != nullptr, "Required child of id " + to_string(id) + " was not found in object " + string(*this));

  return child;
}

shared_ptr<WorldObject> WorldObject::RequireChild(string name)
{
  auto child = GetChild(name);

  Assert(child != nullptr, "Required child of name \"" + name + "\" was not found in object " + string(*this));

  return child;
}

shared_ptr<WorldObject> WorldObject::GetChild(int id)
{
  if (children.count(id) == 0)
    return nullptr;

  auto child = children[id].lock();

  if (child == nullptr)
    children.erase(id);

  return child;
}

shared_ptr<WorldObject> WorldObject::GetChild(string name)
{
  // For each child entry
  auto childEntryIterator = children.begin();
  while (childEntryIterator != children.end())
  {
    // If it's expired
    if (childEntryIterator->second.expired())
    {
      // Remove it
      childEntryIterator = children.erase(childEntryIterator);

      continue;
    }

    // Otherwise check if this is it
    auto child = childEntryIterator->second.lock();
    if (child->GetName() == name)
      return child;

    // Advance
    childEntryIterator++;
  }

  return nullptr;
}

auto WorldObject::InternalDestroy() -> unordered_map<int, weak_ptr<WorldObject>>::iterator
{
  // cout << "Destroying " << *this << endl;
  // cout << "Children: " << endl;
  // for (auto [childId, weakChild] : children)
  // {
  //   LOCK(weakChild, child);

  //   cout << "Child " << *child << endl;
  // }

  // Remove all children
  auto pairIterator = children.begin();
  while (pairIterator != children.end())
  {
    LOCK(pairIterator->second, child);

    pairIterator = child->InternalDestroy();
  }

  // Wrap all components up
  for (
      auto componentIterator = components.begin();
      componentIterator != components.end();
      componentIterator = RemoveComponent(componentIterator->second))
    ;

  // Get pointer to self
  auto shared = GetShared();

  // Remove this object's reference from it's parent
  unordered_map<int, weak_ptr<WorldObject>>::iterator iterator;

  if (IsRoot() == false)
    iterator = UnlinkParent();

  // Delete self from scene's list
  GetScene()->RemoveObject(id);

  // Ensure no more references to self than the one in this function and the one which called this function
  Assert(shared.use_count() <= 2, "Found " + to_string(shared.use_count() - 2) + " leaked references to world object " + GetName() + " when trying to destroy it");

  return iterator;
}

void WorldObject::OnCollision(Collision::Data collisionData)
{
  // Register collision
  frameCollisions[collisionData.GetHash()] = collisionData;

  // Alert all components
  for (auto [componentId, component] : components)
    component->OnCollision(collisionData);
}

void WorldObject::OnCollisionEnter(Collision::Data collisionData)
{
  // Alert all components
  for (auto [componentId, component] : components)
    component->OnCollisionEnter(collisionData);
}

void WorldObject::OnCollisionExit(Collision::Data collisionData)
{
  // Alert all components
  for (auto [componentId, component] : components)
    component->OnCollisionExit(collisionData);
}

void WorldObject::OnTriggerCollision(TriggerCollisionData triggerData)
{
  // Register trigger
  frameTriggers[triggerData.GetHash()] = triggerData;

  // Alert all components
  for (auto [componentId, component] : components)
    component->OnTriggerCollision(triggerData);
}

void WorldObject::OnTriggerCollisionEnter(TriggerCollisionData triggerData)
{
  // Alert all components
  for (auto [componentId, component] : components)
    component->OnTriggerCollisionEnter(triggerData);
}

void WorldObject::OnTriggerCollisionExit(TriggerCollisionData triggerData)
{
  // Alert all components
  for (auto [componentId, component] : components)
    component->OnTriggerCollisionExit(triggerData);
}

void WorldObject::DontDestroyOnLoad(bool value)
{
  if (InternalGetParent()->IsRoot() == false)
  {
    cout << "WARNING: Tried to set non-root object to not destroy on load" << endl;
    return;
  }

  keepOnLoad = value;
}

shared_ptr<GameScene> WorldObject::GetScene()
{
  auto currentScene = Game::GetInstance().GetScene();

  Assert(gameSceneId == currentScene->id, "Trying to access scene of object which is not in the current scene");

  return currentScene;
}

bool WorldObject::IsDescendantOf(const WorldObject &other) const
{
  if (id == other.id)
    return true;

  if (GetParent() == nullptr)
    return false;

  return GetParent()->IsDescendantOf(other);
}

bool WorldObject::SameLineage(const WorldObject &first, const WorldObject &second)
{
  return first.IsDescendantOf(second) || second.IsDescendantOf(first);
}

void WorldObject::SetPhysicsLayer(PhysicsLayer newLayer)
{
  Assert(newLayer != PhysicsLayer::None, "Setting layer to None is forbidden; use Default for irrelevant objects");

  physicsLayer = newLayer;
  inheritedPhysicsLayer = false;
}

PhysicsLayer WorldObject::GetPhysicsLayer() { return physicsLayer; }

bool WorldObject::IsCollidingWith(shared_ptr<Collider> collider)
{
  for (auto [collisionHash, collision] : frameCollisions)
    IF_LOCK(collision.weakOther, other)
    {
      if (other->id == collider->id)
        return true;
    }

  return false;
}

bool WorldObject::WasCollidingWith(shared_ptr<Collider> collider)
{
  for (auto [collisionHash, collision] : lastFrameCollisions)
    IF_LOCK(collision.weakOther, other)
    {
      if (other->id == collider->id)
        return true;
    }

  return false;
}

bool WorldObject::IsTriggerCollidingWith(shared_ptr<Collider> collider)
{
  for (auto [triggerHash, triggerData] : frameTriggers)
    IF_LOCK(triggerData.weakOther, other)
    {
      if (other->id == collider->id)
        return true;
    }

  return false;
}

bool WorldObject::WasTriggerCollidingWith(shared_ptr<Collider> collider)
{
  for (auto [triggerHash, triggerData] : lastFrameTriggers)
    IF_LOCK(triggerData.weakOther, other)
    {
      if (other->id == collider->id)
        return true;
    }

  return false;
}

bool WorldObject::CollisionDealtWith(Collision::Data collisionData)
{
  return frameCollisions.count(collisionData.GetHash()) > 0;
}
bool WorldObject::CollisionDealtWithLastFrame(Collision::Data collisionData)
{
  return lastFrameCollisions.count(collisionData.GetHash()) > 0;
}

bool WorldObject::TriggerCollisionDealtWith(TriggerCollisionData triggerData)
{
  return frameTriggers.count(triggerData.GetHash()) > 0;
}
bool WorldObject::TriggerCollisionDealtWithLastFrame(TriggerCollisionData triggerData)
{
  return lastFrameTriggers.count(triggerData.GetHash()) > 0;
}

bool WorldObject::operator==(const WorldObject &other) const { return other.id == id; }

WorldObject::operator string() const
{
  return "[" + GetName() + "::" + to_string(id) + "]";
}

ostream &operator<<(ostream &stream, const WorldObject &object)
{
  stream << (string)object;
  return stream;
}

bool WorldObject::IsEnabled() const
{
  if (enabled == false)
    return false;

  if (IsRoot())
    return true;

  return InternalGetParent()->IsEnabled();
}

void WorldObject::TriggerDelayedFunctions()
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

int WorldObject::DelayFunction(function<void()> procedure, float seconds)
{
  // Get token id
  int tokenId = HashTwo(SDL_GetTicks(), RandomRange(0, 10000));

  // Store function
  delayedFunctions[tokenId] = {procedure, true};

  // Start timer
  timer.Reset(to_string(tokenId), -seconds);

  return tokenId;
}

void WorldObject::CancelDelayedFunction(int tokenId)
{
  // Set it as not supposed to be called
  delayedFunctions[tokenId].second = false;
}

bool WorldObject::DestroyRequested() const { return destroyRequested; }

void WorldObject::RequestDestroy() { SetEnabled(false), destroyRequested = true; }
