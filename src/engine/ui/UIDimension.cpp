#include "UIDimension.h"

using namespace std;

UIDimension::UIDimension(Axis axis, shared_ptr<UIObject> owner, UnitType type, float value)
    : axis(axis), value(value), type(type), weakOwner(owner) {}

size_t UIDimension::AsRealPixels() const { return AsRealPixels(Default); }

size_t UIDimension::AsRealPixels(Calculation) const
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

float UIDimension::As(UnitType requestedType) const
{
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

DirectedDimension::DirectedDimension(std::shared_ptr<UIObject> owner)
    : top(UIDimension::Vertical, owner),
      right(UIDimension::Horizontal, owner),
      bottom(UIDimension::Vertical, owner),
      left(UIDimension::Horizontal, owner) {}
