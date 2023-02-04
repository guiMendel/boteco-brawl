#include "UIDimension.h"
#include "Game.h"

using namespace std;
using namespace Helper;

UIDimension4::UIDimension4()
    : top(UIDimension::Vertical),
      right(UIDimension::Horizontal),
      bottom(UIDimension::Vertical),
      left(UIDimension::Horizontal)
{
  auto raiseOwn = [this](size_t, size_t)
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

  cout << "top and left: " << top.AsRealPixels() << ", " << left.AsRealPixels() << endl;
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

size_t UIDimension4::SumAlong(UIDimension::Axis axis)
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

  cout << "x and y: " << x.AsRealPixels() << ", " << y.AsRealPixels() << endl;
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

UIDimension::UIDimension(Axis axis) : axis(axis), type(RealPixels) {}

size_t UIDimension::AsRealPixels() { return AsRealPixels(Default); }

size_t UIDimension::AsRealPixels(Calculation configuration)
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
  return CalculateRealPixelSize(configuration);
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

float UIDimension::RealPixelsTo(size_t valuePixels, UnitType requestedType) const
{
  // Catch happy case
  if (requestedType == RealPixels)
    return valuePixels;

  // If arrived here we have some error
  throw runtime_error("ERROR: unrecognized UIDimension unit type");
}

void UIDimension::Set(UnitType newType, float newValue)
{
  type = newType;
  value = newValue;
}

void UIDimension::SetOwner(std::shared_ptr<UIObject> owner) { weakOwner = owner; }

UIDimension::Axis UIDimension::GetCrossAxis(Axis axis)
{
  return axis == Horizontal ? Vertical : Horizontal;
}

size_t UIDimension::CalculateRealPixelSize(Calculation configuration) const
{
  // Catch happy case
  if (type == RealPixels)
    return value;

  // When in percent
  // if (type == Percent)
  // {
  //   // Get owner's parent dimensions, ignoring depending children (such as this one) to avoid a paradox
  //   float parentSize = Lock(weakOwner)->GetParent()->GetSize(axis).AsRealPixels(configuration | IgnoreDependentChildren);

  //   // Return percentage applied to this size
  //   return size_t(parentSize * value / 100);
  // }

  // When in auto
  // if (type == Auto)
  // {
  //   // Get content box
  //   auto contentBox = Lock(weakOwner)->GetContentBox(configuration);

  //   // Use content box size
  //   return contentBox.GetSize(axis);
  // }

  // If arrived here we have some error
  throw runtime_error("ERROR: unrecognized UIDimension unit type");
}

void UIDimension::PrecalculateDefault()
{
  auto newSize = CalculateRealPixelSize(Default);

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
