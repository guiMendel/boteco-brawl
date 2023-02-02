#include "UIContainer.h"

using namespace std;

UIContainer::UIContainer(Canvas &canvas, string name, shared_ptr<UIContainer> parent)
    : UIObject(canvas, name, parent) {}

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
