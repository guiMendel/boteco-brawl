#include <algorithm>
#include "GameObject.h"
#include "Sound.h"
#include "Game.h"
#include <iostream>

using namespace std;

// Private constructor
GameObject::GameObject(string name, int gameStateId, int id)
    : id(id >= 0 ? id : Game::GetInstance().SupplyId()), name(name), gameStateId(gameStateId)
{
}

// With dimensions
GameObject::GameObject(string name, Vector2 coordinates, double rotation, shared_ptr<GameObject> parent)
    : GameObject(name, Game::GetInstance().GetState()->id)
{
  // Add gameState reference
  auto gameState = GetState();
  auto shared = gameState->RegisterObject(this);

  // Only add a parent if not the root object
  if (IsRoot() == false)
  {
    // If no parent, add root as parent
    if (parent == nullptr)
      parent = gameState->GetRootObject();

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

// Allows for registering to the state's variables
void GameObject::RegisterToState()
{
  auto currentState = GetState()->id;

  if (lastStateRegisteredTo == currentState)
    return;

  lastStateRegisteredTo = currentState;

  for (auto [componentId, component] : components)
    component->RegisterToStateWithLayer();
}

void GameObject::Update(float deltaTime)
{
  // Apply timescale
  deltaTime *= timeScale;

  // Update timers
  timer.Update(deltaTime);

  // Trigger delayed functions
  auto delayedFunctionEntryIterator = delayedFunctions.begin();
  while (delayedFunctionEntryIterator != delayedFunctions.end())
  {
    auto tokenId = to_string(delayedFunctionEntryIterator->first);

    // Check if delay is up
    if (timer.Get(tokenId) >= 0)
    {
      auto &delayedFunction = delayedFunctionEntryIterator->second;

      // Scrap timer
      timer.Scrap(tokenId);

      // Trigger function
      delayedFunction();

      // Forget entry
      delayedFunctionEntryIterator = delayedFunctions.erase(delayedFunctionEntryIterator);
    }

    else
      delayedFunctionEntryIterator++;
  }

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

  deltaTime *= timeScale;

  // Check for collision & trigger exit
  DetectCollisionExits();

  for (auto [componentId, component] : components)
  {
    if (component->IsEnabled())
      component->PhysicsUpdate(deltaTime);
  }
}

void GameObject::DetectCollisionExits()
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

void GameObject::OnStatePause()
{
  for (auto [componentId, component] : components)
    component->OnStatePause();
}

void GameObject::OnStateResume()
{
  for (auto [componentId, component] : components)
    component->OnStateResume();
}

decltype(GameObject::components)::iterator GameObject::RemoveComponent(shared_ptr<Component> component)
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

void GameObject::HandleColliderDestruction(shared_ptr<Component> component)
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

shared_ptr<GameObject> GameObject::GetShared()
{
  return GetState()->GetObject(id);
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

shared_ptr<GameObject> GameObject::InternalGetParent() const
{
  // Ensure not root
  Assert(IsRoot() == false, "Getting parent is forbidden on root object");

  if (IsRoot())
    return nullptr;

  // Ensure the parent is there
  Assert(weakParent.expired() == false, "GameObject " + name + " unexpectedly failed to retrieve parent object");

  return weakParent.lock();
}

shared_ptr<GameObject> GameObject::GetParent() const
{
  auto parent = InternalGetParent();

  return parent->id == 0 ? nullptr : parent;
}

void GameObject::UnlinkParent()
{
  if (IsRoot())
    return;

  InternalGetParent()->children.erase(id);
  weakParent.reset();
}

void GameObject::SetParent(shared_ptr<GameObject> newParent)
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
Vector2 GameObject::GetPosition()
{
  if (IsRoot())
    return localPosition;

  // if (GetName() == "DashParticles")
  //   cout << localPosition << endl;

  return InternalGetParent()->GetPosition() + localPosition;
}
void GameObject::SetPosition(const Vector2 newPosition)
{
  if (IsRoot())
    localPosition = newPosition;
  localPosition = newPosition - InternalGetParent()->GetPosition();
}

void GameObject::Translate(const Vector2 translation)
{
  SetPosition(GetPosition() + translation);
}

// Absolute scale of the object
Vector2 GameObject::GetScale()
{
  if (IsRoot())
    return localScale;
  return InternalGetParent()->GetScale() * localScale;
}
void GameObject::SetScale(const Vector2 newScale)
{
  if (IsRoot())
    localScale = newScale;
  localScale = newScale - InternalGetParent()->GetScale();
}

// Absolute rotation in radians
double GameObject::GetRotation()
{
  if (IsRoot())
    return localRotation;
  return InternalGetParent()->GetRotation() + localRotation;
}
void GameObject::SetRotation(const double newRotation)
{
  if (IsRoot())
    localRotation = newRotation;
  localRotation = newRotation - InternalGetParent()->GetRotation();
}

shared_ptr<GameObject> GameObject::CreateChild(string name)
{
  return CreateChild(name, Vector2::Zero(), 0);
}

shared_ptr<GameObject> GameObject::CreateChild(string name, Vector2 offset)
{
  return CreateChild(name, offset, 0);
}

shared_ptr<GameObject> GameObject::CreateChild(string name, Vector2 offset, float offsetRotation)
{
  auto childId = (new GameObject(name, GetPosition() + offset, GetRotation() + offsetRotation, GetShared()))->id;
  return GetState()->GetObject(childId);
}

vector<shared_ptr<GameObject>> GameObject::GetChildren()
{
  vector<shared_ptr<GameObject>> verifiedChildren;

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

shared_ptr<GameObject> GameObject::RequireChild(int id)
{
  auto child = GetChild(id);

  // cout << "Children:" << endl;
  // for (auto child : children)
  //   cout << *child.second.lock() << endl;

  Assert(child != nullptr, "Required child of id " + to_string(id) + " was not found in object " + string(*this));

  return child;
}

shared_ptr<GameObject> GameObject::RequireChild(string name)
{
  auto child = GetChild(name);

  Assert(child != nullptr, "Required child of name \"" + name + "\" was not found in object " + string(*this));

  return child;
}

shared_ptr<GameObject> GameObject::GetChild(int id)
{
  if (children.count(id) == 0)
    return nullptr;

  auto child = children[id].lock();

  if (child == nullptr)
    children.erase(id);

  return child;
}

shared_ptr<GameObject> GameObject::GetChild(string name)
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

void GameObject::InternalDestroy()
{
  // Wrap all components up
  for (
      auto componentIterator = components.begin();
      componentIterator != components.end();
      componentIterator = RemoveComponent(componentIterator->second))
    ;

  // Get pointer to self
  auto shared = GetShared();

  // Remove all children
  for (auto pairIterator = children.begin(); pairIterator != children.end(); ++pairIterator)
  {
    LOCK(pairIterator->second, child);

    child->InternalDestroy();
  }

  // Remove this object's reference from it's parent
  UnlinkParent();

  // Delete self from state's list
  GetState()->RemoveObject(id);

  // Ensure no more references to self than the one in this function and the one which called this function
  Assert(shared.use_count() == 2, "Found leaked references to game object " + GetName() + " when trying to destroy it");
}

void GameObject::OnCollision(Collision::Data collisionData)
{
  // Register collision
  frameCollisions[collisionData.GetHash()] = collisionData;

  // Alert all components
  for (auto [componentId, component] : components)
    component->OnCollision(collisionData);
}

void GameObject::OnCollisionEnter(Collision::Data collisionData)
{
  // Alert all components
  for (auto [componentId, component] : components)
    component->OnCollisionEnter(collisionData);
}

void GameObject::OnCollisionExit(Collision::Data collisionData)
{
  // Alert all components
  for (auto [componentId, component] : components)
    component->OnCollisionExit(collisionData);
}

void GameObject::OnTriggerCollision(TriggerCollisionData triggerData)
{
  // Register trigger
  frameTriggers[triggerData.GetHash()] = triggerData;

  // Alert all components
  for (auto [componentId, component] : components)
    component->OnTriggerCollision(triggerData);
}

void GameObject::OnTriggerCollisionEnter(TriggerCollisionData triggerData)
{
  // Alert all components
  for (auto [componentId, component] : components)
    component->OnTriggerCollisionEnter(triggerData);
}

void GameObject::OnTriggerCollisionExit(TriggerCollisionData triggerData)
{
  // Alert all components
  for (auto [componentId, component] : components)
    component->OnTriggerCollisionExit(triggerData);
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

shared_ptr<GameState> GameObject::GetState()
{
  auto currentState = Game::GetInstance().GetState();

  Assert(gameStateId == currentState->id, "Trying to access state of object which is not in the current state");

  return currentState;
}

bool GameObject::IsDescendantOf(const GameObject &other) const
{
  if (id == other.id)
    return true;

  if (GetParent() == nullptr)
    return false;

  return GetParent()->IsDescendantOf(other);
}

bool GameObject::SameLineage(const GameObject &first, const GameObject &second)
{
  return first.IsDescendantOf(second) || second.IsDescendantOf(first);
}

void GameObject::SetPhysicsLayer(PhysicsLayer newLayer)
{
  Assert(newLayer != PhysicsLayer::None, "Setting layer to None is forbidden; use Default for irrelevant objects");

  physicsLayer = newLayer;
  inheritedPhysicsLayer = false;
}

PhysicsLayer GameObject::GetPhysicsLayer() { return physicsLayer; }

bool GameObject::IsCollidingWith(shared_ptr<Collider> collider)
{
  for (auto [collisionHash, collision] : frameCollisions)
    IF_LOCK(collision.weakOther, other)
    {
      if (other->id == collider->id)
        return true;
    }

  return false;
}

bool GameObject::WasCollidingWith(shared_ptr<Collider> collider)
{
  for (auto [collisionHash, collision] : lastFrameCollisions)
    IF_LOCK(collision.weakOther, other)
    {
      if (other->id == collider->id)
        return true;
    }

  return false;
}

bool GameObject::IsTriggerCollidingWith(shared_ptr<Collider> collider)
{
  for (auto [triggerHash, triggerData] : frameTriggers)
    IF_LOCK(triggerData.weakOther, other)
    {
      if (other->id == collider->id)
        return true;
    }

  return false;
}

bool GameObject::WasTriggerCollidingWith(shared_ptr<Collider> collider)
{
  for (auto [triggerHash, triggerData] : lastFrameTriggers)
    IF_LOCK(triggerData.weakOther, other)
    {
      if (other->id == collider->id)
        return true;
    }

  return false;
}

bool GameObject::CollisionDealtWith(Collision::Data collisionData)
{
  return frameCollisions.count(collisionData.GetHash()) > 0;
}
bool GameObject::CollisionDealtWithLastFrame(Collision::Data collisionData)
{
  return lastFrameCollisions.count(collisionData.GetHash()) > 0;
}

bool GameObject::TriggerCollisionDealtWith(TriggerCollisionData triggerData)
{
  return frameTriggers.count(triggerData.GetHash()) > 0;
}
bool GameObject::TriggerCollisionDealtWithLastFrame(TriggerCollisionData triggerData)
{
  return lastFrameTriggers.count(triggerData.GetHash()) > 0;
}

bool GameObject::operator==(const GameObject &other) const { return other.id == id; }

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

int GameObject::DelayFunction(function<void()> procedure, float seconds)
{
  // Get token id
  int tokenId = RandomRange(0, 1000000);

  // Store function
  delayedFunctions[tokenId] = procedure;

  // Start timer
  timer.Reset(to_string(tokenId), -seconds);
}

void GameObject::CancelDelayedFunction(int tokenId)
{
  delayedFunctions.erase(tokenId);
  timer.Scrap(to_string(tokenId));
}

void GameObject::SetTimeScale(float newScale)
{
  timeScale = newScale;
}

float GameObject::GetTimeScale() const { return timeScale; }
