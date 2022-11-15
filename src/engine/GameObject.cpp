#include <algorithm>
#include "GameObject.h"
#include "Sound.h"
#include "Game.h"
#include <iostream>

using namespace std;

// Private constructor
GameObject::GameObject(string name, GameState &gameState) : gameState(gameState), id(gameState.SupplyObjectId()), name(name)
{
}

// With dimensions
GameObject::GameObject(string name, Vector2 coordinates, double rotation, shared_ptr<GameObject> parent)
    : GameObject(name, Game::GetInstance().GetState())
{
  // Add gameState reference
  auto shared = gameState.RegisterObject(this);

  // Only add a parent if not the root object
  if (IsRoot() == false)
  {
    // If no parent, add root as parent
    if (parent == nullptr)
      parent = gameState.GetRootObject();

    // Add reference to parent
    this->weakParent = parent;

    // Give parent a reference to self
    parent->children[id] = weak_ptr(shared);
  }

  SetPosition(coordinates);
  SetRotation(rotation);
}

GameObject::~GameObject()
{
}

void GameObject::Start()
{
  if (started)
    return;

  started = true;

  for (auto component : components)
    component->StartAndRegisterLayer();
}

void GameObject::Update(float deltaTime)
{
  // Update timers
  timer.Update(deltaTime);

  if (enabled == false)
    return;

  for (const auto &component : components)
  {
    if (component->IsEnabled())
      component->Update(deltaTime);
  }
}

void GameObject::OnStatePause()
{
  for (auto component : components)
    component->OnStatePause();
}

void GameObject::OnStateResume()
{
  for (auto component : components)
    component->OnStateResume();
}

void GameObject::RemoveComponent(shared_ptr<Component> component)
{
  // Find it's position
  auto componentPosition = find_if(
      components.begin(), components.end(), [component](shared_ptr<Component> otherComponent)
      { return otherComponent == component; });

  // Detect if not present
  if (componentPosition == components.end())
    return;

  // Wrap it up
  (*componentPosition)->OnBeforeDestroy();

  // Remove it
  components.erase(componentPosition);
}

shared_ptr<GameObject> GameObject::GetShared() const
{
  return gameState.GetObject(id);
}

auto GameObject::GetComponent(const Component *componentPointer) const -> shared_ptr<Component>
{
  auto componentIterator = find_if(
      components.begin(), components.end(),
      [componentPointer](shared_ptr<Component> component)
      { return component.get() == componentPointer; });

  if (componentIterator == components.end())
    return shared_ptr<Component>();

  return *componentIterator;
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
}

// Where this object exists in game space, in absolute coordinates
Vector2 GameObject::GetPosition() const
{
  if (IsRoot())
    return localPosition;
  return InternalGetParent()->GetPosition() + localPosition;
}
void GameObject::SetPosition(const Vector2 newPosition)
{
  if (IsRoot())
    localPosition = newPosition;
  localPosition = newPosition - InternalGetParent()->GetPosition();
}

// Absolute scale of the object
Vector2 GameObject::GetScale() const
{
  if (IsRoot())
    return localScale;
  return InternalGetParent()->GetScale() + localScale;
}
void GameObject::SetScale(const Vector2 newScale)
{
  if (IsRoot())
    localScale = newScale;
  localScale = newScale - InternalGetParent()->GetScale();
}

// Absolute rotation in radians
double GameObject::GetRotation() const
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

void GameObject::InternalDestroy()
{
  // Wrap all components up
  for (auto &component : components)
    component->OnBeforeDestroy();

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
  gameState.RemoveObject(id);

  // Ensure no more references to self than the one in this function and the one which called this function
  Assert(shared.use_count() == 2, "Found leaked references to game object " + GetName() + " when trying to destroy it");
}

void GameObject::OnCollision(GameObject &other)
{
  // Alert all components
  for (auto component : components)
    component->OnCollision(other);
}