#include "ComponentOwner.h"
#include "Component.h"

using namespace std;

auto ComponentOwner::GetComponent(const Component *componentPointer) -> shared_ptr<Component>
{
  if (components.count(componentPointer->id) == 0)
    return nullptr;

  return components[componentPointer->id];
}

auto ComponentOwner::RequireComponent(const Component *componentPointer) -> shared_ptr<Component>
{
  auto component = GetComponent(componentPointer);

  if (!component)
  {
    throw runtime_error(string("Required component was not found"));
  }

  return component;
}

decltype(ComponentOwner::components)::iterator ComponentOwner::RemoveComponent(shared_ptr<Component> component)
{
  // Detect not present
  if (components.count(component->id) == 0)
    return components.end();

  // Wrap it up
  component->OnBeforeDestroy();

  // Remove it
  return components.erase(components.find(component->id));
}