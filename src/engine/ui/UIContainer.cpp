#include "UIContainer.h"

using namespace std;

UIContainer::UIContainer(Canvas &canvas, shared_ptr<UIContainer> parent, string name)
    : UIObject(canvas, parent, name) {}

UIContainer::UIContainer(Canvas &canvas, string name)
    : UIObject(canvas, name) {}

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

std::shared_ptr<GameObject> UIContainer::InternalGetParent() const
{
  if (IsCanvasRoot())
    return GetScene()->RequireGameObject(canvas.gameObject.id);

  return GetParent();
}

std::shared_ptr<UIContainer> UIContainer::GetShared() { return GetScene()->RequireUIObject<UIContainer>(id); }
