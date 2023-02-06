#ifndef __UI_DIMENSION__
#define __UI_DIMENSION__

#include <memory>
#include "Event.h"
#include "Helper.h"
#include "Vector2.h"

class UIObject;
class UIContainer;
class UIFlexboxProperties;
class UIDimension2;
class UIDimension4;

class UIDimension
{
  friend class UIDimension2;
  friend class UIDimension4;
  friend class UIObject;
  friend class UIContainer;

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

  // Raised when the real pixel size of this dimensions changes
  // Provides new and old values for the size
  EventII<size_t, size_t> OnRealPixelSizeChange;

  // Default constructor
  UIDimension(Axis axis);

  virtual ~UIDimension() {}

  // Set a new value for this dimension
  void Set(UnitType type, float value = 0);

  // Get this dimensions specifically as real pixels
  size_t AsRealPixels();

  // Get this dimension in the specified type
  float As(UnitType requestedType);

  // Returns the opposite axis
  static Axis GetCrossAxis(Axis axis);

  // Returns the Vector2 value corresponding to the provided axis
  static float VectorAxis(Vector2 vector, Axis axis);

  // Axis of this dimension
  const Axis axis;

  // Gets value that uniquely identifies this dimension's configuration
  size_t GetHash() const;

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
  size_t AsRealPixels(Calculation configuration);

  // Converts a value in real pixels to the requested value
  float RealPixelsTo(size_t valuePixels, UnitType requestedType) const;

  // Sets the owner
  void SetOwner(std::shared_ptr<UIObject> weakOwner);

  // Calculates dimension in real pixels
  size_t CalculateRealPixelSize(Calculation configuration) const;

  // Calculates and stores real pixel size with default configuration
  // Raises event if size change is detected from last calculation
  void PrecalculateDefault();

  // Last calculated real pixel size
  size_t lastRealPixelSize{0};

  // Frame in which lastRealPixelSize was calculated
  unsigned long precalculationFrame{std::numeric_limits<long>::max()};

  // Internal dimension's value
  float value{0};

  // Internal dimension's unit type
  UnitType type;

  // Owner of this dimension
  std::weak_ptr<UIObject> weakOwner;
};

// Defines a kind of dimension that has 2 values, horizontal and vertical
struct UIDimension2
{
  friend class UIObject;
  friend class UIFlexboxProperties;

  UIDimension x;
  UIDimension y;

  UIDimension2();

  // Sets value for all directions
  void Set(UIDimension::UnitType type, float value = 0);

  // Get value of an axis
  UIDimension &Along(UIDimension::Axis axis);

  // Gets value that uniquely identifies this dimension's configuration
  size_t GetHash() const;

private:
  // Sets the owner
  void SetOwner(std::shared_ptr<UIObject> owner);

  // Calculates and stores real pixel size with default configuration
  // Raises event if size change is detected from last calculation
  void PrecalculateDefault();
};

// Defines a kind of dimension that has 4 values, directed towards the top, right, bottom or left
struct UIDimension4
{
  friend class UIObject;

  UIDimension top;
  UIDimension right;
  UIDimension bottom;
  UIDimension left;

  // Raised when the real pixel size of one of the dimensions changes
  Event OnRealPixelSizeChange;

  UIDimension4();

  // Sets value for all directions
  void Set(UIDimension::UnitType type, float value = 0);

  // Sets value for horizontal directions
  void SetHorizontal(UIDimension::UnitType type, float value = 0);

  // Sets value for vertical directions
  void SetVertical(UIDimension::UnitType type, float value = 0);

  // Get values along an axis (top and left come first)
  std::pair<UIDimension &, UIDimension &> Along(UIDimension::Axis axis);

  // Get summed real pixel values along an axis
  size_t SumAlong(UIDimension::Axis axis);

  // Gets value that uniquely identifies this dimension's configuration
  size_t GetHash() const;

private:
  // Sets the owner
  void SetOwner(std::shared_ptr<UIObject> weakOwner);

  // Calculates and stores real pixel size with default configuration
  // Raises event if size change is detected from last calculation
  void PrecalculateDefault();
};

#endif