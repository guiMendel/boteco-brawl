#include "UIChildrenBox.h"

using namespace std;

UIChildrenGroup::UIChildrenGroup(UIChildrenBox &box) : box(box) {}

void UIChildrenGroup::AllocateChildren(ChildIterator &childIterator, ChildIterator endIterator)
{
  auto &properties = box.GetOwner()->Flexbox();
  auto mainAxis = properties.mainAxis;

  // Safecheck
  Assert(childIterator != endIterator, "UI Children Group Received invalid iterator");

  // Max main size this group can have
  size_t maxMainSize = numeric_limits<size_t>::max();

  // Allocate at least one child
  do
    AllocateChild(childIterator++);
  while (
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

  cout << *box.GetOwner() << " gap: " << properties.gap.Along(mainAxis).AsRealPixels() << endl;

  // Get child
  auto child = (*childIterator);

  // Allocate it to current position
  childrenPositions.push_back(mainSize);

  // Update main size with this object's size + margin + gap
  mainSize += child->GetRealPixelsAlong(mainAxis, true) + properties.gap.Along(mainAxis).AsRealPixels();

  // Keep biggest cross size
  crossSize = max(crossSize, child->GetRealPixelsAlong(crossAxis, true));
}

void UIChildrenBox::SetOwner(shared_ptr<UIContainer> owner) { weakOwner = owner; }

void UIChildrenBox::Recalculate()
{
  LOCK(weakOwner, owner);

  // Clear current groups
  groups.clear();

  // Get cross gap
  auto &properties = owner->Flexbox();
  auto crossGap = properties.gap.Along(properties.mainAxis).AsRealPixels();

  // Reset dimensions
  mainSize = 0;

  // Discount extra gap
  crossSize = -crossGap;

  // Get children iterator
  auto children = owner->GetChildren();
  auto childIterator = children.begin();

  // Stop if no children
  if (children.empty())
    return;

  // Construct groups as they are filled
  while (childIterator != children.end())
    groups.emplace_back(*this).AllocateChildren(childIterator, children.end());

  // Calculate dimensions anew
  for (auto &group : groups)
  {
    mainSize = max(mainSize, group.mainSize);
    crossSize += group.crossSize + crossGap;
  }
}

void UIChildrenBox::RepositionChildren()
{
  LOCK(weakOwner, owner);

  // Get the children
  auto children = owner->GetChildren();

  // Get main axis
  auto mainAxis = owner->properties.mainAxis;
  auto crossAxis = UIDimension::GetCrossAxis(mainAxis);

  // Get an iterator for the children
  auto childIterator = children.begin();

  // Current cross position
  size_t crossPosition{0};

  // For each group
  for (auto &group : groups)
  {
    // For each child position
    for (size_t mainPosition : group.childrenPositions)
    {
      // Ensure iterator is valid
      Assert(childIterator != children.end(), "UI Children Box found less children than it had expected");

      // Get child
      auto child = *childIterator++;

      // Get margins for this child
      auto mainMargin = child->margin.Along(mainAxis).first.AsRealPixels();
      auto crossMargin = child->margin.Along(crossAxis).first.AsRealPixels();

      // Set it's position and increment iterator
      child->SetLocalPositionAlong(mainAxis, mainPosition + mainMargin, crossPosition + crossMargin);
    }

    // Increment cross position
    crossPosition += group.crossSize;
  }

  // Ensure all children have been repositioned
  Assert(childIterator == children.end(), "UI Children Box found more children than it had expected");
}

shared_ptr<UIContainer> UIChildrenBox::GetOwner() const { return Lock(weakOwner); }
