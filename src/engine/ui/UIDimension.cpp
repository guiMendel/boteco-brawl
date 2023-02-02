#include "UIDimension.h"

using namespace std;

UIDirectedDimension::UIDirectedDimension()
    : top(UIDimension::Vertical),
      right(UIDimension::Horizontal),
      bottom(UIDimension::Vertical),
      left(UIDimension::Horizontal) {}

void UIDirectedDimension::Set(UIDimension::UnitType type, float value)
{
  top.Set(type, value);
  right.Set(type, value);
  bottom.Set(type, value);
  left.Set(type, value);
}

void UIDirectedDimension::SetHorizontal(UIDimension::UnitType type, float value)
{
  right.Set(type, value);
  left.Set(type, value);
}

void UIDirectedDimension::SetVertical(UIDimension::UnitType type, float value)
{
  top.Set(type, value);
  bottom.Set(type, value);
}

void UIDirectedDimension::SetOwner(std::shared_ptr<UIObject> owner)
{
  top.SetOwner(owner);
  right.SetOwner(owner);
  bottom.SetOwner(owner);
  left.SetOwner(owner);
}

UIDimension::UIDimension(Axis axis) : axis(axis), type(RealPixels) {}

size_t UIDimension::AsRealPixels() const { return AsRealPixels(Default); }

size_t UIDimension::AsRealPixels(Calculation) const
{
  Assert(weakOwner.expired() == false, "Tried reading value of UI Dimensions without first giving it an owner");

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

float UIDimension::As(UnitType requestedType) const
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
