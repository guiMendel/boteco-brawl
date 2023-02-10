#include "UIChildrenBox.h"

using namespace std;

UIChildrenGroup::UIChildrenGroup(UIChildrenBox &box) : box(box) {}

void UIChildrenGroup::AllocateChildren(ChildIterator &childIterator, ChildIterator endIterator)
{
  auto &properties = box.GetOwner()->Flexbox();
  auto mainAxis = properties.mainAxis;

  // Safecheck
  Assert(childIterator < endIterator, "UI Children Group Received invalid iterator");

  // Max main size this group can have
  int maxMainSize = GetMaxMainSize();

  // Allocate at least one child
  do
  {
    AllocateChild(childIterator);
    childIterator = box.FindValidChild(++childIterator, endIterator);
  } while (
      // Check if there are more children
      childIterator != endIterator &&
      // As well as if next child fits in this group
      mainSize + (*childIterator)->GetDimension(mainAxis).AsRealPixels() <= maxMainSize);

  // Remove extra gap at the end
  mainSize -= properties.gap.Along(mainAxis).AsRealPixels();
}

void UIChildrenGroup::AllocateChild(ChildIterator childIterator)
{
  auto &properties = box.GetOwner()->Flexbox();
  auto mainAxis = properties.mainAxis;
  auto crossAxis = UIDimension::GetCrossAxis(mainAxis);

  // Get child
  auto child = (*childIterator);

  auto childCrossSize = child->GetRealPixelsAlong(crossAxis, true, true);

  // Allocate it to current position
  items.push_back({mainSize, childCrossSize});

  // Update main size with this object's size + margin + gap
  mainSize += child->GetRealPixelsAlong(mainAxis, true, true) + properties.gap.Along(mainAxis).AsRealPixels();

  // Keep biggest cross size
  crossSize = max(crossSize, childCrossSize);
}

int UIChildrenGroup::GetMaxMainSize() const
{
  auto owner = box.GetOwner();

  // If wrap is off, no limit
  if (owner->Flexbox().wrap == false)
    return numeric_limits<int>::max();

  auto mainAxis = owner->Flexbox().mainAxis;
  auto mainDimension = owner->GetDimension(mainAxis);

  // For max content owners
  if (mainDimension.GetType() == UIDimension::MaxContent)
  {
    // Return max dimension size
    return mainDimension.GetMaxSize();
  }

  // For min content owners, get the maximum main size of it's children
  else if (mainDimension.GetType() == UIDimension::MinContent)
  {
    int maxChildSize{0};

    for (auto child : owner->GetChildren())
      maxChildSize = max(maxChildSize, child->GetRealPixelsAlong(mainAxis, true, true));

    // And also ensure it does not exceed the max size
    return min(maxChildSize, mainDimension.GetMaxSize());
  }

  // Any other cases, just return the main size of the owner
  else
    return owner->GetRealPixelsAlong(mainAxis, false);
}

void UIChildrenBox::SetOwner(shared_ptr<UIContainer> owner) { weakOwner = owner; }

void UIChildrenBox::Recalculate()
{
  LOCK(weakOwner, owner);

  // Clear current groups
  groups.clear();

  // Get cross gap
  auto &properties = owner->Flexbox();
  auto crossGap = properties.gap.Along(UIDimension::GetCrossAxis(properties.mainAxis)).AsRealPixels();

  // Reset dimensions
  mainSize = 0;

  // Discount extra gap
  crossSize = 0;

  // Get children iterator
  auto children = owner->GetChildren();

  auto childIterator = FindValidChild(children.begin(), children.end());

  // Construct groups as they are filled
  while (childIterator != children.end())
    groups.emplace_back(*this).AllocateChildren(childIterator, children.end());

  // Calculate dimensions anew
  for (auto &group : groups)
  {
    mainSize = max(mainSize, int(group.mainSize));
    crossSize += group.crossSize + crossGap;
  }

  // Discount extra gap
  if (groups.size() > 0)
    crossSize -= crossGap;
}

void UIChildrenBox::RepositionChildren()
{
  LOCK(weakOwner, owner);

  // Get main axis
  auto mainAxis = owner->properties.mainAxis;
  auto crossAxis = UIDimension::GetCrossAxis(mainAxis);

  // Get children box position relative to container's unpadded size
  // int mainBoxOffset = 0;
  // int crossBoxOffset = 0;
  int mainBoxOffset = owner->GetRealPixelsAlong(mainAxis, false);
  int crossBoxOffset = owner->GetRealPixelsAlong(crossAxis, false) - crossSize;

  // Get the children
  auto children = owner->GetChildren();

  // Get item alignment
  auto alignment = owner->properties.placeItems;

  // Get an iterator for the children
  auto childIterator = FindValidChild(children.begin(), children.end());

  // Current cross position
  int crossPosition{0};

  // Cross gap
  int crossGap = owner->properties.gap.Along(crossAxis).AsRealPixels();

  // For each group
  for (auto &group : groups)
  {
    // Get group empty space before first item
    int mainEmptyOffset = int(mainBoxOffset - group.mainSize) * UIDimension::VectorAxis(alignment, mainAxis);
    // cout << UIDimension::VectorAxis(alignment, mainAxis) << endl;

    // For each child position
    for (auto [mainPosition, itemCrossSize] : group.items)
    {
      // Ensure iterator is valid
      Assert(childIterator != children.end(), "UI Children Box found less children than it had expected");

      // Get empty space of item in cross axis
      int crossEmptyOffset = int(crossBoxOffset + group.crossSize - itemCrossSize) * UIDimension::VectorAxis(alignment, crossAxis);

      // Get child
      auto child = *childIterator;

      // Get margins for this child
      auto mainMargin = child->margin.Along(mainAxis).first.AsRealPixels();
      auto crossMargin = child->margin.Along(crossAxis).first.AsRealPixels();

      // Set it's position and increment iterator
      child->SetLocalPositionAlong(mainAxis,
                                   mainPosition + mainMargin + mainEmptyOffset,
                                   crossPosition + crossMargin + crossEmptyOffset);

      // cout << *child << " position: " << child->GetPosition() << ", size: " << child->width.AsRealPixels() << ", " << child->height.AsRealPixels() << endl;

      //  Get next child
      childIterator = FindValidChild(++childIterator, children.end());
    }

    // Increment cross position
    crossPosition += group.crossSize + crossGap;
  }

  // Ensure all children have been repositioned
  Assert(childIterator == children.end(), "UI Children Box found more children than it had expected");
}

shared_ptr<UIContainer> UIChildrenBox::GetOwner() const { return Lock(weakOwner); }

int UIChildrenBox::GetRealPixelsAlong(UIDimension::Axis axis)
{
  if (axis == Lock(weakOwner)->properties.mainAxis)
    return mainSize;
  else
    return crossSize;
}

UIChildrenBox::ChildIterator UIChildrenBox::FindValidChild(ChildIterator childIterator, ChildIterator endIterator)
{
  if (childIterator == endIterator)
    return endIterator;

  auto mainAxis = Lock(weakOwner)->Flexbox().mainAxis;
  auto crossAxis = UIDimension::GetCrossAxis(mainAxis);

  auto isInvalid = [this, mainAxis, crossAxis](shared_ptr<UIObject> child)
  {
    auto &childMain = child->GetDimension(mainAxis);
    if (ignoreDependentChildrenMain &&
        (childMain.GetType() == UIDimension::Percent ||
         childMain.GetMaxType() == UIDimension::Percent ||
         childMain.GetMinType() == UIDimension::Percent))
      return true;

    auto &childCross = child->GetDimension(crossAxis);
    if (ignoreDependentChildrenCross &&
        (childCross.GetType() == UIDimension::Percent ||
         childCross.GetMaxType() == UIDimension::Percent ||
         childCross.GetMinType() == UIDimension::Percent))
      return true;

    if (child->IsPositionAbsolute())
      return true;

    return false;
  };

  while (childIterator != endIterator && isInvalid(*childIterator))
    childIterator++;

  return childIterator;
}
