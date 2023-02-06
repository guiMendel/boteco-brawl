#ifndef __UI_CHILDREN_BOX__
#define __UI_CHILDREN_BOX__

#include "UIDimension.h"
#include <vector>

struct UIFlexboxProperties;
struct UIContainer;
class UIChildrenBox;

// One main axis aligned group where the children within which the children will actually be distributed
struct UIChildrenGroup
{
  using ChildIterator = std::vector<std::shared_ptr<UIObject>>::iterator;

  UIChildrenGroup(UIChildrenBox &box);

  // Course through the given iterator as long as the children still fit in this group
  // Update the group with the allocated children
  // When no more children fit, stop
  void AllocateChildren(ChildIterator &childIterator, ChildIterator endIterator);

  // Allocates one child from iterator (doesn't check for iterator validity)
  void AllocateChild(ChildIterator childIterator);

  // Real pixel size of the group along the container's main axis
  size_t mainSize{0};

  // Real pixel size of the group along the container's cross axis
  size_t crossSize{0};

  // Guards position in main axis & size in cross axis of each item inside this group (real pixels)
  std::vector<std::pair<size_t, size_t>> items{};

private:
  // Get the max main size of this group
  size_t GetMaxMainSize() const;

  // Reference to owner box
  UIChildrenBox &box;
};

// An abstract representation of the box that bounds a UI Container's children, which holds all groups with children
class UIChildrenBox
{
  friend struct UIContainer;
  friend struct UIChildrenGroup;

public:
  using ChildIterator = UIChildrenGroup::ChildIterator;

  // Sets the owner of this box
  void SetOwner(std::shared_ptr<UIContainer> owner);

  // Recalculates the values of this box using it's parent's children and flexbox properties
  void Recalculate();

  // Reposition the owner's children according to the current value of this box
  void RepositionChildren();

  // Get real pixel size along an axis
  size_t GetRealPixelsAlong(UIDimension::Axis axis);

  std::shared_ptr<UIContainer> GetOwner() const;

private:
  // Returns the next iterator that points to a child that doesn't depend on it's paren't size
  ChildIterator FindIndependent(ChildIterator childIterator, ChildIterator endIterator);

  // Real pixel size of the box along the container's main axis
  size_t mainSize{0};

  // Real pixel size of the box along the container's cross axis
  size_t crossSize{0};

  // Whether to ignore owner's children that depend on it's size
  bool ignoreDependentChildrenMain{false}, ignoreDependentChildrenCross{false};

  // All groups which together compose this box
  std::vector<UIChildrenGroup> groups{};

  // Reference to owner container
  std::weak_ptr<UIContainer> weakOwner;
};

#include "UIContainer.h"

#endif