#include "UIContainer.h"

using namespace std;

size_t UIFlexboxProperties::GetHash() const
{
  // Dont' take reverse into consideration, as it doesn't affect the content box calculation
  return size_t(mainAxis);
}

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

shared_ptr<GameObject> UIContainer::InternalGetParent() const
{
  if (IsCanvasRoot())
    return GetScene()->RequireGameObject(canvas.gameObject.id);

  return GetParent();
}

shared_ptr<UIContainer> UIContainer::GetShared() { return GetScene()->RequireUIObject<UIContainer>(id); }

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

  // Must return true iff first parameter comes before second parameter
  auto comparer = [](shared_ptr<UIObject> object1, shared_ptr<UIObject> object2)
  {
    return object1->arrangeOrder < object2->arrangeOrder;
  };

  // Sort them
  sort(children.begin(), children.end(), comparer);

  return children;
}
