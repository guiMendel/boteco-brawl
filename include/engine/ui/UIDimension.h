#ifndef __UI_DIMENSION__
#define __UI_DIMENSION__

#include <memory>

class UIObject;

class UIDimension
{
public:
  // Defines the possible axis for a dimension
  enum Axis
  {
    Horizontal,
    Vertical
  };

  // Defines the possible unit types
  enum UnitType
  {
    // Size equals the size of the owner's content box
    // Auto,

    // Constant dimension size
    RealPixels

    // Size that depends on parent's size for this dimension
    // Is calculated based on the parent's size BEFORE it grows on account of children that depend on it's size (such as this one)
    // Percent
  };

  // Default constructor
  UIDimension(Axis axis, std::shared_ptr<UIObject> owner, UnitType type = RealPixels, float value = 0);

  virtual ~UIDimension() {}

  // Set a new value for this dimension
  void Set(UnitType type, float value = 0);

  // Get this dimensions specifically as real pixels
  size_t AsRealPixels() const;

  // Get this dimension in the specified type
  float As(UnitType requestedType) const;

  // Axis of this dimension
  const Axis axis;

private:
  // Defines compatible configurations for the calculation of dimensions
  enum Calculation
  {
    // Default calculation
    Default = 0b00

    // When calculating with a type that depends on the children's sizes of the same dimension,
    // will consider any children's size that depend on THIS object's size as 0
    // IgnoreDependentChildren = 0b01
  };

  // Get this dimensions specifically as real pixels
  // Allows configuring how calculation is done
  size_t AsRealPixels(Calculation configuration) const;

  // Converts a value in real pixels to the requested value
  float RealPixelsTo(size_t valuePixels, UnitType requestedType) const;

  // Internal dimension's value
  float value;

  // Internal dimension's unit type
  UnitType type;

  // Owner of this dimension
  const std::weak_ptr<UIObject> weakOwner;
};

// Defines a kind of dimension that has 4 values, directed towards the top, right, bottom or left
struct UIDirectedDimension
{
  UIDimension top;
  UIDimension right;
  UIDimension bottom;
  UIDimension left;

  UIDirectedDimension(std::shared_ptr<UIObject> owner);

  // Sets value for all directions
  void Set(UIDimension::UnitType type, float value = 0);
  
  // Sets value for horizontal directions
  void SetHorizontal(UIDimension::UnitType type, float value = 0);

  // Sets value for vertical directions
  void SetVertical(UIDimension::UnitType type, float value = 0);
};

#include "UIObject.h"

#endif