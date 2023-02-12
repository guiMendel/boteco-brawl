#include "UIContent.h"
#include "UIContainer.h"

using namespace std;

UIContent::UIContent(Canvas &canvas, string name, std::shared_ptr<UIContainer> parent)
    : UIObject(canvas, name, parent) {}

void UIContent::CascadeDown(function<void(GameObject &)> callback, bool topDown)
{
  // Execute on this object
  if (topDown)
    callback(*this);

  // Execute on it's components
  for (auto [componentId, component] : components)
    component->CascadeDown(callback, topDown);

  // Execute on this object (bottom up case)
  if (topDown == false)
    callback(*this);
}

std::shared_ptr<GameObject> UIContent::InternalGetParentNoException() const { return GetParent(); }

void UIContent::CheckEventApplication(std::shared_ptr<UIMouseEvent> mouseEvent)
{
  if (Contains(mouseEvent->screenPosition))
    // Raise in this object if still propagating
    if (mouseEvent->propagating)
      RaiseMouseEvent(mouseEvent);
}
