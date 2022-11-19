#include <map>
#include <string>
#include <utility>
#include <list>
#include <functional>
#include <memory>
#include "GameObject.h"
#include "ComponentParameter.h"

/* COMPONENT CLASS INCLUDES */

#define MAKE_INSERTER(className) [](shared_ptr<GameObject> gameObject, map<string, ComponentParameter> params) \
{ gameObject->AddComponent<className>(params); }

using namespace std;

// A map of a component param's name to it's type
using component_param_types = map<string, ComponentParameterType>;

// Type of function that adds a component to an object, given it's params
using component_inserter = function<void(shared_ptr<GameObject>, map<string, ComponentParameter>)>;

// A table linking each component's class name to a map of it's param names to types, and also a function to add this component to a given GameObject (function also takes a map of param names to values)
using component_table = map<
    string, pair<component_param_types, component_inserter>>;

// Returns the components table
auto getComponentsTable() -> component_table
{
  // Initialize empty table
  component_table table;

  // Will temporarily hold the param types for each component
  component_param_types paramTypes;

  /* ADD COMPONENTS TO TABLE */
}