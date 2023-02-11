#ifndef __COMPONENT_OWNER__
#define __COMPONENT_OWNER__

#include "Helper.h"
#include <unordered_map>
#include <functional>
#include <string>

class Component;

class ComponentOwner
{
protected:
  // Map with all components of this object, indexed by the component's ids
  std::unordered_map<int, std::shared_ptr<Component>> components;

public:
  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto GetComponent() -> std::shared_ptr<T>
  {
    // Find the position of the component that is of the requested type
    auto componentIterator = std::find_if(
        components.begin(), components.end(), [](std::pair<int, std::shared_ptr<Component>> componentEntry)
        { return dynamic_cast<T *>(componentEntry.second.get()) != nullptr; });

    // Detect if not present
    if (componentIterator == components.end())
      return nullptr;

    return Helper::RequirePointerCast<T>(componentIterator->second);
  }

  // Gets pointer to a component of the given type
  // Needs to be in header file so the compiler knows how to build the necessary methods
  template <class T>
  auto GetComponents() -> std::vector<std::shared_ptr<T>>
  {
    std::vector<std::shared_ptr<T>> foundComponents;

    for (auto [componentId, component] : components)
    {
      if (dynamic_cast<T *>(component.get()) != nullptr)
        foundComponents.push_back(Helper::RequirePointerCast<T>(component));
    }

    return foundComponents;
  }

  // Like GetComponent, but raises if it's not present
  template <class T>
  auto RequireComponent() -> std::shared_ptr<T>
  {
    auto component = GetComponent<T>();

    if (!component)
    {
      throw std::runtime_error(std::string("Required component was not found.\nRequired component typeid name: ") + typeid(T).name());
    }

    return component;
  }

  // Like GetComponents, but raises if it finds none
  template <class T>
  auto RequireComponents() -> std::shared_ptr<T>
  {
    auto components = GetComponents<T>();

    if (!components)
    {
      throw std::runtime_error(std::string("Required component was not found.\nRequired component typeid name: ") + typeid(T).name());
    }

    return components;
  }

  auto GetComponent(const Component *componentPointer) -> std::shared_ptr<Component>;
  auto RequireComponent(const Component *componentPointer) -> std::shared_ptr<Component>;

  auto GetComponent(int id) -> std::shared_ptr<Component>;
  auto RequireComponent(int id) -> std::shared_ptr<Component>;

  // Removes an existing component
  decltype(components)::iterator RemoveComponent(std::shared_ptr<Component> component);
};

#endif
