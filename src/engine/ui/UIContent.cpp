#include "UIContent.h"
#include "UIContainer.h"

using namespace std;

UIContent::UIContent(Canvas &canvas, shared_ptr<UIContainer> parent, string name)
    : UIObject(canvas, parent, name) {}

UIContent::UIContent(Canvas &canvas, string name)
    : UIObject(canvas, name) {}

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

std::shared_ptr<GameObject> UIContent::InternalGetParent() const { return GetParent(); }
