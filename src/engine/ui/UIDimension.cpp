#include "UIDimension.h"
#include "UIObject.h"
#include "Game.h"

using namespace std;
using namespace Helper;

UIDimension4::UIDimension4()
    : top(UIDimension::Vertical),
      right(UIDimension::Horizontal),
      bottom(UIDimension::Vertical),
      left(UIDimension::Horizontal)
{
  auto raiseOwn = [this](int, int)
  {
    OnRealPixelSizeChange.Invoke();
  };

  top.OnRealPixelSizeChange.AddListener("directed-dimension", raiseOwn);
  right.OnRealPixelSizeChange.AddListener("directed-dimension", raiseOwn);
  bottom.OnRealPixelSizeChange.AddListener("directed-dimension", raiseOwn);
  left.OnRealPixelSizeChange.AddListener("directed-dimension", raiseOwn);
}

void UIDimension4::Set(UIDimension::UnitType type, float value)
{
  top.Set(type, value);
  right.Set(type, value);
  bottom.Set(type, value);
  left.Set(type, value);
}

void UIDimension4::SetHorizontal(UIDimension::UnitType type, float value)
{
  right.Set(type, value);
  left.Set(type, value);
}

void UIDimension4::SetVertical(UIDimension::UnitType type, float value)
{
  top.Set(type, value);
  bottom.Set(type, value);
}

void UIDimension4::SetOwner(std::shared_ptr<UIObject> owner)
{
  top.SetOwner(owner);
  right.SetOwner(owner);
  bottom.SetOwner(owner);
  left.SetOwner(owner);
}

void UIDimension4::PrecalculateDefault()
{
  top.PrecalculateDefault();
  right.PrecalculateDefault();
  bottom.PrecalculateDefault();
  left.PrecalculateDefault();
}

std::pair<UIDimension &, UIDimension &> UIDimension4::Along(UIDimension::Axis axis)
{
  if (axis == UIDimension::Horizontal)
    return {left, right};

  return {top, bottom};
}

int UIDimension4::SumAlong(UIDimension::Axis axis)
{
  auto margins = Along(axis);
  return margins.first.AsRealPixels() + margins.second.AsRealPixels();
}

UIDimension2::UIDimension2()
    : x(UIDimension::Horizontal),
      y(UIDimension::Vertical) {}

void UIDimension2::Set(UIDimension::UnitType type, float value)
{
  x.Set(type, value);
  y.Set(type, value);
}

void UIDimension2::Set(UIDimension::UnitType type, Vector2 value)
{
  x.Set(type, value.x);
  y.Set(type, value.y);
}

void UIDimension2::SetOwner(std::shared_ptr<UIObject> owner)
{
  x.SetOwner(owner);
  y.SetOwner(owner);
}

void UIDimension2::PrecalculateDefault()
{
  x.PrecalculateDefault();
  y.PrecalculateDefault();
}

UIDimension &UIDimension2::Along(UIDimension::Axis axis)
{
  return axis == UIDimension::Horizontal ? x : y;
}

Vector2 UIDimension2::AsVector()
{
  return Vector2(x.AsRealPixels(), y.AsRealPixels());
}

UIDimension::UIDimension(Axis axis, UnitType initialType)
    : axis(axis), type(initialType), maxType(None), minType(None) {}

int UIDimension::AsRealPixels() { return AsRealPixels(Default); }

int UIDimension::AsRealPixels(Calculation configuration)
{
  Assert(weakOwner.expired() == false, "Tried reading value of UI Dimensions without first giving it an owner");

  // When no config is set, use last calculation if it's still valid
  if (configuration == Default)
  {
    // Check if storage is valid
    if (precalculationFrame != Game::currentFrame)
      PrecalculateDefault();

    return lastRealPixelSize;
  }

  // Otherwise, perform calculation
  return CalculateRealPixelSize(value, type, configuration);
}

float UIDimension::As(UnitType requestedType)
{
  Assert(weakOwner.expired() == false, "Tried reading value of UI Dimensions without first giving it an owner");

  // Catch happy case
  if (type == requestedType)
    return value;

  // Get value in real pixels
  // Convert real pixels to requested type
  return RealPixelsTo(AsRealPixels(), requestedType);
}

float UIDimension::RealPixelsTo(int valuePixels, UnitType requestedType) const
{
  // Catch happy case
  if (requestedType == RealPixels)
    return valuePixels;

  // Camera dependent
  if (requestedType == WorldUnits)
    return valuePixels * Lock(Lock(weakOwner)->canvas.weakCamera)->GetUnitsPerRealPixel();

  // Parent dependent
  if (requestedType == Percent)
  {
    // Get owner's parent dimensions, ignoring depending children (such as this one) to avoid a paradox
    auto parent = Lock(weakOwner)->RequireParent();
    Calculation ignoreChildren = axis == Horizontal ? IgnoreDependentChildrenX : IgnoreDependentChildrenY;
    int parentSize = parent->GetDimension(axis).AsRealPixels(UIDimension::Calculation(ignoreChildren));

    // Return percentage applied to this size
    return int(float(valuePixels) / float(parentSize) * 100);
  }

  // Catch valueless target types
  if (requestedType == None || requestedType == MaxContent || requestedType == MinContent)
  {
    MESSAGE << "WARNING: tried converting UI Dimensions real pixels to a valueless type" << endl;
    return 0;
  }

  // If arrived here we have some error
  throw runtime_error("unrecognized UIDimension unit type");
}

void UIDimension::Set(UnitType newType, float newValue)
{
  type = newType;
  value = newValue;

  // Invalidate calculation cache
  precalculationFrame = Game::currentFrame - 1;
}

void UIDimension::SetMax(UnitType newType, float newValue)
{
  maxType = newType;
  maxValue = newValue;

  // Invalidate calculation cache
  precalculationFrame = Game::currentFrame - 1;
}

void UIDimension::SetMin(UnitType newType, float newValue)
{
  minType = newType;
  minValue = newValue;

  // Invalidate calculation cache
  precalculationFrame = Game::currentFrame - 1;
}

void UIDimension::SetOwner(std::shared_ptr<UIObject> owner)
{
  weakOwner = owner;

  // Invalidate calculation cache
  precalculationFrame = Game::currentFrame - 1;
}

UIDimension::Axis UIDimension::GetCrossAxis(Axis axis)
{
  return axis == Horizontal ? Vertical : Horizontal;
}

int UIDimension::CalculateRealPixelSize(float value, UnitType type, Calculation configuration, bool clamp) const
{
  // Min & max values
  int minPixels{numeric_limits<int>::min()}, maxPixels{numeric_limits<int>::max()};

  if (clamp)
  {
    minPixels = GetMinSize(configuration);
    maxPixels = GetMaxSize(configuration);
  }

  // Catch happy case
  if (type == RealPixels)
    return Clamp(int(value), minPixels, maxPixels);

  // Catch happy case
  if (type == None)
    return 0;

  // Camera dependent
  if (type == WorldUnits)
    return Clamp(int(value * Lock(Lock(weakOwner)->canvas.weakCamera)->GetRealPixelsPerUnit()), minPixels, maxPixels);

  // Catch content-dependent cases
  if (type == MaxContent || type == MinContent)
    return Clamp(Lock(weakOwner)->GetContentRealPixelsAlong(axis, configuration), minPixels, maxPixels);

  // When in percent
  if (type == Percent)
  {
    // Get owner's parent dimensions, ignoring depending children (such as this one) to avoid a paradox
    auto parent = Lock(weakOwner)->RequireParent();
    Calculation ignoreChildren = axis == Horizontal ? IgnoreDependentChildrenX : IgnoreDependentChildrenY;
    int parentSize = parent->GetDimension(axis).AsRealPixels(UIDimension::Calculation(configuration | ignoreChildren));

    // Return percentage applied to this size
    return Clamp(int(float(parentSize) * value / 100), minPixels, maxPixels);
  }

  // If arrived here we have some error
  throw runtime_error("unrecognized UIDimension unit type");
}

void UIDimension::PrecalculateDefault()
{
  auto newSize = CalculateRealPixelSize(value, type, Default);

  // Compare
  if (newSize == lastRealPixelSize)
    return;

  // Raise
  OnRealPixelSizeChange.Invoke(newSize, lastRealPixelSize);

  // Update it
  lastRealPixelSize = newSize;

  // Record current frame
  precalculationFrame = Game::currentFrame;
}

int UIDimension::GetHash() const { return HashTwo(int(axis), int(value * 100)); }
int UIDimension2::GetHash() const { return HashTwo(x.GetHash(), y.GetHash()); }
int UIDimension4::GetHash() const { return HashMany(top.GetHash(), right.GetHash(), bottom.GetHash(), left.GetHash()); }

float UIDimension::VectorAxis(Vector2 vector, Axis axis)
{
  return axis == Horizontal ? vector.x : vector.y;
}

UIDimension::UnitType UIDimension::GetType() const { return type; }

UIDimension::UnitType UIDimension::GetMaxType() const { return maxType; }

UIDimension::UnitType UIDimension::GetMinType() const { return minType; }

int UIDimension::GetMaxSize() const { return GetMaxSize(Default); }

int UIDimension::GetMaxSize(Calculation configuration) const
{
  if (maxType == None)
    return numeric_limits<int>::max();

  return CalculateRealPixelSize(maxValue, maxType, configuration, false);
}

int UIDimension::GetMinSize() const { return GetMinSize(Default); }

int UIDimension::GetMinSize(Calculation configuration) const
{
  if (minType == None)
    return numeric_limits<int>::min();

  return CalculateRealPixelSize(minValue, minType, configuration, false);
}
