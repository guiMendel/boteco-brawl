#include <algorithm>
#include "WorldObject.h"
#include "Sound.h"
#include "Game.h"
#include <iostream>

using namespace std;

const float WorldObject::objectCollectionRange{50};

// Private constructor
WorldObject::WorldObject(string name, int gameSceneId, int id) : GameObject(name, gameSceneId, id) {}

// With dimensions
WorldObject::WorldObject(string name, Vector2 coordinates, double rotation, shared_ptr<WorldObject> parent)
    : GameObject(name)
{
  // Only add a parent if not the root object
  if (IsRoot() == false)
  {
    // If no parent, add root as parent
    if (parent == nullptr)
      parent = GetScene()->GetRootObject();

    // Add reference to parent
    this->weakParent = parent;

    // Parent will get reference to this object upon it's registration to the scene
    // That's because this object's shared pointer hasn't been created yet!
  }

  // Inherit parent's layer (don't use SetPhysicsLayer here, as we don't want to set inheritedPhysicsLayer to false)
  physicsLayer = parent->physicsLayer;

  SetPosition(coordinates);
  SetRotation(rotation);
}

WorldObject::~WorldObject() {}

void WorldObject::Update(float deltaTime)
{
  // Usual updates
  GameObject::Update(deltaTime);

  // Check range collection
  auto absolutePosition = GetPosition().GetAbsolute();

  if (absolutePosition.x > objectCollectionRange || absolutePosition.y > objectCollectionRange)
  {
    MESSAGE << "Collecting " << *this << " for exceeding collection range" << endl;

    RequestDestroy();
  }
}

void WorldObject::PhysicsUpdate(float deltaTime)
{
  // Usual updates
  GameObject::PhysicsUpdate(deltaTime);

  // Check for collision & trigger exit
  if (IsEnabled())
    DetectCollisionExits();
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

void WorldObject::HandleColliderDestruction(shared_ptr<Collider> collider)
{
  // Trigger collisions to exit
  unordered_map<size_t, TriggerCollisionData> exitTriggers;

  // For each of this frame's triggers
  for (auto [triggerHash, triggerData] : frameTriggers)
  {
    // Raise if the source collider is this one
    IF_LOCK(triggerData.weakSource, source)
    IF_LOCK(triggerData.weakOther, other)
    if (source->id == collider->id)
      exitTriggers[triggerHash] = triggerData;
  }

  // For each of last frame's triggers
  for (auto [triggerHash, triggerData] : lastFrameTriggers)
  {
    // Raise if the source collider is this one
    IF_LOCK(triggerData.weakSource, source)
    IF_LOCK(triggerData.weakOther, other)
    if (source->id == collider->id)
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
    if (source->id == collider->id)
      exitCollisions[collisionHash] = collisionData;
  }

  // For each of last frame's collisions
  for (auto [collisionHash, collisionData] : lastFrameCollisions)
  {
    // Raise if the source collider is this one
    IF_LOCK(collisionData.weakSource, source)
    IF_LOCK(collisionData.weakOther, other)
    if (source->id == collider->id)
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
  if (IsRoot())
    return GetScene()->GetRootObject();

  return GetScene()->RequireWorldObject(id);
}

shared_ptr<GameObject> WorldObject::InternalGetParentNoException() const
{
  return weakParent.lock();
}

std::shared_ptr<WorldObject> WorldObject::InternalGetWorldParent() const
{
  // Ensure not root
  Assert(IsRoot() == false, "Getting parent is forbidden on root object");

  return RequirePointerCast<WorldObject>(InternalGetParentNoException());
}

shared_ptr<WorldObject> WorldObject::GetParent() const
{
  return IsRoot() ? nullptr : InternalGetWorldParent();
}

auto WorldObject::UnlinkParent() -> unordered_map<int, weak_ptr<WorldObject>>::iterator
{
  Assert(IsRoot() == false, "Root has no parent to unlink");

  // Get parent
  auto parent = InternalGetWorldParent();

  // Get own iterator
  auto iterator = parent->children.find(id);

  // Remove it
  if (iterator != parent->children.end())
    iterator = parent->children.erase(iterator);
  weakParent.reset();

  return iterator;
}

void WorldObject::SetParent(shared_ptr<WorldObject> newParent, std::shared_ptr<WorldObject> ownPointer)
{
  Assert(IsRoot() == false, "SetParent is forbidden on root object");

  if (ownPointer == nullptr)
    ownPointer = GetShared();

  // Delete current parent
  if (weakParent.expired() == false)
    UnlinkParent();

  // Set new parent
  weakParent = newParent;
  newParent->children[id] = ownPointer;

  // Inherit this new parent's layer if necessary
  if (inheritedPhysicsLayer)
    physicsLayer = newParent->physicsLayer;
}

void WorldObject::SetParent(shared_ptr<WorldObject> newParent)
{
  SetParent(newParent, GetShared());
}

// Where this object exists in game space, in absolute coordinates
Vector2 WorldObject::GetPosition()
{
  if (IsRoot())
    return localPosition;

  return InternalGetWorldParent()->GetPosition() + localPosition;
}
void WorldObject::SetPosition(const Vector2 newPosition)
{
  if (IsRoot())
    localPosition = newPosition;
  localPosition = newPosition - InternalGetWorldParent()->GetPosition();
}

void WorldObject::Translate(const Vector2 translation)
{
  SetPosition(GetPosition() + translation);
}

// Absolute scale of the object
Vector2 WorldObject::GetScale()
{
  if (IsRoot())
    return localScale;
  return InternalGetWorldParent()->GetScale() * localScale;
}
void WorldObject::SetScale(const Vector2 newScale)
{
  if (IsRoot())
    localScale = newScale;

  Vector2 parentScale{InternalGetWorldParent()->GetScale()};

  Assert(parentScale.x > 0 && parentScale.y > 0, "Parent scale had a 0 is invalid");

  localScale = newScale / parentScale;
}

// Absolute rotation in radians
double WorldObject::GetRotation()
{
  if (IsRoot())
    return localRotation;
  return InternalGetWorldParent()->GetRotation() + localRotation;
}
void WorldObject::SetRotation(const double newRotation)
{
  if (IsRoot())
    localRotation = newRotation;
  localRotation = newRotation - InternalGetWorldParent()->GetRotation();
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
  return GetScene()->NewObject<WorldObject>(name, GetPosition() + offset, GetRotation() + offsetRotation, GetShared());
}

void WorldObject::InternalDestroy() { DestroySelf(); }

auto WorldObject::DestroySelf() -> std::unordered_map<int, std::weak_ptr<WorldObject>>::iterator
{
  // cout << "Destroying " << *this << endl;
  // cout << "Children: " << endl;
  // for (auto [childId, weakChild] : children)
  // {
  //   LOCK(weakChild, child);

  //   cout << "Child " << *child << endl;
  // }

  // Store pointer to self for later
  auto shared = GetShared();

  // Remove all children
  auto pairIterator = children.begin();
  while (pairIterator != children.end())
  {
    LOCK(pairIterator->second, child);

    pairIterator = child->DestroySelf();
  }

  // Call base destroy method to clean up components & remove reference from scene
  GameObject::InternalDestroy();

  // Remove this object's reference from it's parent
  unordered_map<int, weak_ptr<WorldObject>>::iterator iterator;

  if (IsRoot() == false && weakParent.expired() == false)
    iterator = UnlinkParent();

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
    RequirePointerCast<WorldComponent>(component)->OnCollision(collisionData);
}

void WorldObject::OnCollisionEnter(Collision::Data collisionData)
{
  // Alert all components
  for (auto [componentId, component] : components)
    RequirePointerCast<WorldComponent>(component)->OnCollisionEnter(collisionData);
}

void WorldObject::OnCollisionExit(Collision::Data collisionData)
{
  // Alert all components
  for (auto [componentId, component] : components)
    RequirePointerCast<WorldComponent>(component)->OnCollisionExit(collisionData);
}

void WorldObject::OnTriggerCollision(TriggerCollisionData triggerData)
{
  // Register trigger
  frameTriggers[triggerData.GetHash()] = triggerData;

  // Alert all components
  for (auto [componentId, component] : components)
    RequirePointerCast<WorldComponent>(component)->OnTriggerCollision(triggerData);
}

void WorldObject::OnTriggerCollisionEnter(TriggerCollisionData triggerData)
{
  // Alert all components
  for (auto [componentId, component] : components)
    RequirePointerCast<WorldComponent>(component)->OnTriggerCollisionEnter(triggerData);
}

void WorldObject::OnTriggerCollisionExit(TriggerCollisionData triggerData)
{
  // Alert all components
  for (auto [componentId, component] : components)
    RequirePointerCast<WorldComponent>(component)->OnTriggerCollisionExit(triggerData);
}

bool WorldObject::IsDescendantOf(std::shared_ptr<WorldObject> other) const
{
  if (*this == *other)
    return true;

  if (GetParent() == nullptr)
    return false;

  return GetParent()->IsDescendantOf(other);
}

bool WorldObject::SameLineage(
    std::shared_ptr<WorldObject> first, std::shared_ptr<WorldObject> second)
{
  return first->IsDescendantOf(second) || second->IsDescendantOf(first);
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

void WorldObject::CascadeDown(function<void(GameObject &)> callback, bool topDown)
{
  // Execute on this object
  if (topDown)
    callback(*this);

  // Execute on it's children
  for (auto child : GetChildren())
    child->CascadeDown(callback, topDown);

  // Execute on it's components
  for (auto [componentId, component] : components)
    component->CascadeDown(callback, topDown);

  // Execute on this object (bottom up case)
  if (topDown == false)
    callback(*this);
}

void WorldObject::InternalSetParent(std::shared_ptr<GameObject> newParent, std::shared_ptr<GameObject> ownPointer)
{
  if (ownPointer == nullptr)
    ownPointer = GetShared();

  SetParent(RequirePointerCast<WorldObject>(newParent), RequirePointerCast<WorldObject>(ownPointer));
}
