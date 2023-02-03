#ifndef __UI_CHILDREN_BOX__
#define __UI_CHILDREN_BOX__

#include "UIDimension.h"
#include <vector>

struct UIFlexboxProperties;
struct UIContainer;

// One main axis aligned group where the children within which the children will actually be distributed
struct UIChildrenGroup
{
  using ChildIterator = std::vector<std::shared_ptr<UIObject>>::iterator;

  UIChildrenGroup(UIDimension::Axis mainAxis);

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

  // Position of this group's items along the main axis, in real pixels
  std::vector<size_t> childrenPositions{};

private:
  UIDimension::Axis mainAxis;
  UIDimension::Axis crossAxis;
};

// An abstract representation of the box that bounds a UI Container's children, which holds all groups with children
class UIChildrenBox
{
public:
  // Sets the owner of this box
  void SetOwner(std::shared_ptr<UIContainer> owner);

  // Recalculates the values of this box using it's parent's children and flexbox properties
  void Recalculate();

  // Reposition the owner's children according to the current value of this box
  void RepositionChildren();

private:
  // Real pixel size of the box along the container's main axis
  size_t mainSize{0};

  // Real pixel size of the box along the container's cross axis
  size_t crossSize{0};

  // All groups which together compose this box
  std::vector<UIChildrenGroup> groups{};

  // Reference to owner container
  std::weak_ptr<UIContainer> weakOwner;
};

#include "UIContainer.h"

#endif