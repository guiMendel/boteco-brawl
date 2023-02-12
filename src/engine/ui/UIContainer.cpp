#include "UIContainer.h"

using namespace std;

int UIFlexboxProperties::GetHash() const
{
  // Dont' take reverse into consideration, as it doesn't affect the content box calculation
  return HashMany(int(mainAxis),
                  gap.GetHash(),
                  int(placeItems.x * 100),
                  int(placeItems.y * 100));
}

void UIFlexboxProperties::SetOwner(std::shared_ptr<UIObject> owner) { gap.SetOwner(owner); }

UIContainer::UIContainer(
    Canvas &canvas,
    string name,
    shared_ptr<UIContainer> parent,
    UIFlexboxProperties properties)
    : UIObject(canvas, name, parent),
      properties(properties),
      lastPropertiesHash(properties.GetHash()) {}

void UIContainer::Awake()
{
  childrenBox.SetOwner(GetShared());

  UIObject::Awake();
}

void UIContainer::CascadeDown(function<void(GameObject &)> callback, bool topDown)
{
  // Execute on this object
  if (topDown)
    callback(*this);

  // Execute on it's children
  for (auto child : GetChildren())
    child->CascadeDown(callback, topDown);

  // Execute on it's components
  for (auto [componentId, component] : components)
    component->CascadeDown(callback, topDown);

  // Execute on this object (bottom up case)
  if (topDown == false)
    callback(*this);
}

shared_ptr<GameObject> UIContainer::InternalGetParentNoException() const
{
  if (IsCanvasRoot())
    return GetScene()->GetGameObject(canvas.gameObject.id);

  return GetParent();
}

shared_ptr<UIContainer> UIContainer::GetShared() const { return GetScene()->RequireUIObject<UIContainer>(id); }

void UIContainer::InternalDestroy() { DestroySelf(); }

auto UIContainer::DestroySelf() -> unordered_map<int, weak_ptr<UIObject>>::iterator
{
  // cout << *this << "'s children: " << endl;
  // for (auto [childId, weakChild] : children)
  // {
  //   LOCK(weakChild, child);

  //   cout << "Child " << *child << endl;
  // }

  // Remove all children
  auto pairIterator = children.begin();
  while (pairIterator != children.end())
  {
    LOCK(pairIterator->second, child);

    pairIterator = child->DestroySelf();
  }

  return UIObject::DestroySelf();
}

void UIContainer::RecalculateChildrenBox()
{
  // Perform recalculation
  childrenBox.Recalculate();

  // Update children's positions
  childrenBox.RepositionChildren();
}

UIFlexboxProperties &UIContainer::Flexbox()
{
  checkPropertyChange = true;
  return properties;
}

void UIContainer::Update(float deltaTime)
{
  // For root's update
  if (IsCanvasRoot())
  {
    // Precalculate the dimensions of the whole tree
    auto precalculate = [](GameObject &object)
    {
      auto shared = object.GetScene()->RequireUIObject<UIObject>(object.id);

      shared->PrecalculateDimensions();
    };

    // Run through tree bottom up
    CascadeDown(precalculate, false);
  }

  // Detect if recalculation of children box is necessary
  DetectRecalculation();

  UIObject::Update(deltaTime);
}

void UIContainer::DetectRecalculation()
{
  if (
      // Either force is toggled
      forceRecalculation ||
      // Or properties have changed
      (checkPropertyChange && lastPropertiesHash != properties.GetHash()))
  {
    RecalculateChildrenBox();
    checkPropertyChange = false;
    lastPropertiesHash = properties.GetHash();
    forceRecalculation = false;
  }
}

vector<shared_ptr<UIObject>> UIContainer::GetChildren()
{
  // Get children
  auto children = Parent::GetChildren();

  // Whether to revert order
  bool revert = properties.reverseDirection;

  // Must return true iff first parameter comes before second parameter
  auto comparer = [revert](shared_ptr<UIObject> object1, shared_ptr<UIObject> object2)
  {
    if (revert)
      return object1->arrangeOrder > object2->arrangeOrder;

    return object1->arrangeOrder < object2->arrangeOrder;
  };

  // Sort them
  sort(children.begin(), children.end(), comparer);

  return children;
}

void UIContainer::InitializeDimensions()
{
  UIObject::InitializeDimensions();

  properties.SetOwner(GetShared());
}

void UIContainer::PrecalculateDimensions()
{
  UIObject::PrecalculateDimensions();

  properties.gap.x.PrecalculateDefault();
  properties.gap.y.PrecalculateDefault();
}

int UIContainer::GetContentRealPixelsAlong(UIDimension::Axis axis, UIDimension::Calculation config)
{
  if (config == UIDimension::Calculation::Default)
    return childrenBox.GetRealPixelsAlong(axis);

  return GetIndependentContentRealPixels(axis, config);
}

int UIContainer::GetIndependentContentRealPixels(UIDimension::Axis axis, UIDimension::Calculation config) const
{
  // Create a children box to calculate separately
  UIChildrenBox box;

  // Give it a reference to this container
  box.SetOwner(GetShared());

  // Set it to ignore dependent children
  if (config & UIDimension::Calculation::IgnoreDependentChildrenX)
  {
    if (properties.mainAxis == UIDimension::Horizontal)
      box.ignoreDependentChildrenMain = true;
    else
      box.ignoreDependentChildrenCross = true;
  }
  else if (config & UIDimension::Calculation::IgnoreDependentChildrenX)
  {
    if (properties.mainAxis == UIDimension::Vertical)
      box.ignoreDependentChildrenMain = true;
    else
      box.ignoreDependentChildrenCross = true;
  }

  // Calculate the box
  box.Recalculate();

  // Return it's dimensions
  return properties.mainAxis == axis ? box.mainSize : box.crossSize;
}

void UIContainer::CheckEventApplication(std::shared_ptr<UIMouseEvent> mouseEvent)
{
  if (Contains(mouseEvent->screenPosition))
  {
    // Propagate check downwards
    for (auto child : GetChildren())
      child->CheckEventApplication(mouseEvent);

    // Raise in this object if still propagating
    if (mouseEvent->propagating)
      RaiseMouseEvent(mouseEvent);
  }
}
