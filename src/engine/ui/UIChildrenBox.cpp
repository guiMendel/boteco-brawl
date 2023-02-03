#include "UIChildrenBox.h"

using namespace std;

UIChildrenGroup::UIChildrenGroup(UIDimension::Axis mainAxis)
    : mainAxis(mainAxis), crossAxis(UIDimension::GetCrossAxis(mainAxis)) {}

void UIChildrenGroup::AllocateChildren(ChildIterator &childIterator, ChildIterator endIterator)
{
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
      mainSize + (*childIterator)->GetSize(mainAxis).AsRealPixels() <= maxMainSize);
}

void UIChildrenGroup::AllocateChild(ChildIterator childIterator)
{
  // Get child
  auto child = (*childIterator);

  // Allocate it to current position
  childrenPositions.push_back(mainSize);

  // Update main size
  mainSize += child->GetRealPixelsAlong(mainAxis, true);

  // Keep biggest cross size
  crossSize = max(crossSize, child->GetRealPixelsAlong(crossAxis, true));
}

void UIChildrenBox::SetOwner(shared_ptr<UIContainer> owner) { weakOwner = owner; }

void UIChildrenBox::Recalculate()
{
  LOCK(weakOwner, owner);

  // Clear current groups
  groups.clear();

  // Reset dimensions
  mainSize = 0;
  crossSize = 0;

  // Get children iterator
  auto children = owner->GetChildren();
  auto childIterator = children.begin();

  // Stop if no children
  if (children.empty())
    return;

  // Construct groups as they are filled
  while (childIterator != children.end())
    groups.emplace_back(owner->properties.mainAxis).AllocateChildren(childIterator, children.end());

  // Calculate them anew
  for (auto &group : groups)
  {
    mainSize = max(mainSize, group.mainSize);
    crossSize += group.crossSize;
  }
}

void UIChildrenBox::RepositionChildren()
{
  LOCK(weakOwner, owner);

  // Get the children
  auto children = owner->GetChildren();

  // Get main axis
  auto mainAxis = owner->properties.mainAxis;

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

      // Set it's position and increment iterator
      (*childIterator++)->SetLocalPositionAlong(mainAxis, mainPosition, crossPosition);
    }

    // Increment cross position
    crossPosition += group.crossSize;
  }

  // Ensure all children have been repositioned
  Assert(childIterator == children.end(), "UI Children Box found more children than it had expected");
}