#ifndef __PARENT__
#define __PARENT__

#include "Helper.h"
#include "ComponentOwner.h"
#include <unordered_map>
#include <functional>
#include <string>

class Component;

template <typename ChildClass>
class Parent : virtual public ComponentOwner
{
  // =================================
  // OBJECT HIERARCHY
  // =================================
public:
  virtual std::vector<std::shared_ptr<ChildClass>> GetChildren() { return Helper::WeakMapToVector(children); }

  std::shared_ptr<ChildClass> GetChild(std::string name)
  {
    for (auto child : GetChildren())
      if (child->GetName() == name)
        return child;

    return nullptr;
  }

  std::shared_ptr<ChildClass> GetChild(int id)
  {
    if (children.count(id) == 0)
      return nullptr;

    auto child = children[id].lock();

    if (child == nullptr)
      children.erase(id);

    return child;
  }

  std::shared_ptr<ChildClass> RequireChild(std::string name)
  {
    auto child = GetChild(name);

    Assert(child != nullptr, "Required child of name \"" + name + "\" was not found");

    return child;
  }

  std::shared_ptr<ChildClass> RequireChild(int id)
  {
    auto child = GetChild(id);

    Assert(child != nullptr, "Required child of id " + std::to_string(id) + " was not found");

    return child;
  }

  // Child objects
  std::unordered_map<int, std::weak_ptr<ChildClass>> children;

  // =================================
  // COMPONENT HANDLING
  // =================================
public:
  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto GetComponentsInChildren() -> std::vector<std::shared_ptr<T>>
  {
    return InternalGetComponentsInChildren<T>({});
  }

  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto GetComponentInChildren() -> std::shared_ptr<T>
  {
    // Try to get in this object
    std::shared_ptr<T> component = GetComponent<T>();

    // If found, return it
    if (component != nullptr)
      return component;

    // For each child
    for (std::shared_ptr<ChildClass> child : GetChildren())
    {
      // In case it's another parent
      std::shared_ptr<Parent<ChildClass>> childAsParent = std::dynamic_pointer_cast<Parent>(child);

      if (childAsParent)
      {
        component = childAsParent->Parent<ChildClass>::GetComponentInChildren<T>();

        if (component != nullptr)
          return component;
      }

      // If it has no children
      else
      {
        component = child->ComponentOwner::GetComponent<T>();

        if (component != nullptr)
          return component;
      }
    }

    return nullptr;
  }

  // Like GetComponentInChildren, but raises if it's not present
  template <class T>
  auto RequireComponentInChildren() -> std::shared_ptr<T>
  {
    auto component = GetComponentInChildren<T>();

    if (!component)
      throw std::runtime_error(std::string("Required component was not found in children.\nRequired component typeid name: ") + typeid(T).name());

    return component;
  }

private:
  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto InternalGetComponentsInChildren(std::vector<std::shared_ptr<T>> foundComponents) -> std::vector<std::shared_ptr<T>>
  {
    // For each child
    for (auto child : GetChildren())
    {
      auto childAsParent = std::dynamic_pointer_cast<Parent>(child);

      // In case it's another parent
      if (childAsParent)
        foundComponents = childAsParent->InternalGetComponentsInChildren(foundComponents);

      // If it has no children
      else
      {
        std::vector<std::shared_ptr<T>> newComponents = child->ComponentOwner::GetComponents<T>();

        // Merge
        if (newComponents.size() > 0)
          foundComponents.insert(foundComponents.end(), newComponents.begin(), newComponents.end());
      }
    }

    // Merge own components
    std::vector<std::shared_ptr<T>> newComponents = GetComponents<T>();

    // Merge
    if (newComponents.size() > 0)
      foundComponents.insert(foundComponents.end(), newComponents.begin(), newComponents.end());

    return foundComponents;
  }
};

#endif
