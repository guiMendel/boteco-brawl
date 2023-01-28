#include <string>
#include <map>

// Available types for a component parameter
enum class ComponentParameterType
{
  Int,
  Float,
  String,
  Bool
};

// Stores a component parameter's value
union ComponentParameter
{
  int asInt;
  float asFloat;
  // std::string asString;
  bool asBool;
};

// For ease of use
using ComponentParameters = std::map<std::string, ComponentParameter>;