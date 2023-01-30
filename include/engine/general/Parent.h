#ifndef __PARENT__
#define __PARENT__

#include "Helper.h"
#include <unordered_map>
#include <functional>
#include <string>

class Component;

template <typename ChildClass>
class Parent
{
  // =================================
  // OBJECT HIERARCHY
  // =================================
public:
  std::vector<std::shared_ptr<ChildClass>> GetChildren() { return Helper::WeakMapToVector(children); }

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

protected:
  // Executes the given function for this object and then cascades it down to any children it has
  void CascadeDownChildren(std::function<void(ChildClass &)> callback, bool topDown = true)
  {
    // Execute on this object
    if (topDown)
      callback(*this);

    // Execute on it's children
    for (auto child : GetChildren())
      child->CascadeDown(callback, topDown);

    // Execute on this object (bottom up case)
    if (topDown == false)
      callback(*this);
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
    std::shared_ptr<T> component = InternalGetComponent<T>();

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
        component = childAsParent->GetComponentInChildren();

        if (component != nullptr)
          return component;
      }

      // If it has no children
      else
      {
        component = child->InternalGetComponent();

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

protected:
  // Gets all components of this object
  virtual std::vector<std::shared_ptr<Component>> GetAllComponents() const = 0;

  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto InternalGetComponent() -> std::shared_ptr<T>
  {
    auto components = GetAllComponents();

    // Find the position of the component that is of the requested type
    auto componentIterator = std::find_if(
        components.begin(), components.end(), [](std::shared_ptr<Component> component)
        { return dynamic_cast<T *>(component.get()) != nullptr; });

    // Detect if not present
    if (componentIterator == components.end())
      return nullptr;

    return RequirePointerCast<T>(*componentIterator);
  }

  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto InternalGetComponents() -> std::vector<std::shared_ptr<T>>
  {
    auto components = GetAllComponents();

    std::vector<std::shared_ptr<T>> foundComponents;

    for (auto component : components)
      if (dynamic_cast<T *>(component.get()) != nullptr)
        foundComponents.push_back(RequirePointerCast<T>(component));

    return foundComponents;
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
        std::vector<std::shared_ptr<T>> newComponents = child->InternalGetComponents();

        // Merge
        if (newComponents.size() > 0)
          foundComponents.insert(foundComponents.end(), newComponents.begin(), newComponents.end());
      }
    }

    // Merge own components
    std::vector<std::shared_ptr<T>> newComponents = InternalGetComponents();

    // Merge
    if (newComponents.size() > 0)
      foundComponents.insert(foundComponents.end(), newComponents.begin(), newComponents.end());

    return foundComponents;
  }
};

#endif
