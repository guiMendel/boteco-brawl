#include <unordered_map>
#include <string>
#include <utility>
#include <list>
#include <functional>
#include <memory>
#include "GameObject.h"
#include "ComponentParameter.h"

/* COMPONENT CLASS INCLUDES */

#define MAKE_INSERTER(className) [](shared_ptr<GameObject> gameObject, unordered_map<string, ComponentParameter> params) \
{ gameObject->AddComponent<className>(params); }

using namespace std;

// A unordered_map of a component param's name to it's type
using component_param_types = unordered_map<string, ComponentParameterType>;

// Type of function that adds a component to an object, given it's params
using component_inserter = function<void(shared_ptr<GameObject>, unordered_map<string, ComponentParameter>)>;

// A table linking each component's class name to a unordered_map of it's param names to types, and also a function to add this component to a given GameObject (function also takes a unordered_map of param names to values)
using component_table = unordered_map<
    string, pair<component_param_types, component_inserter>>;

// Returns the components table
auto getComponentsTable() -> component_table
{
  // Initialize empty table
  component_table table;

  // Will temporarily hold the param types for each component
  component_param_types paramTypes;

  /* ADD COMPONENTS TO TABLE */

  return table;
}