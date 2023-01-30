#include "UIComponent.h"

using namespace std;

UIComponent::UIComponent(GameObject &associatedObject)
    : Component(associatedObject),
      uiObject(*GetScene()->RequireUIObject<UIObject>(associatedObject.id)) {}

UIComponent::~UIComponent() {}

shared_ptr<UIComponent> UIComponent::GetShared() const
{
  auto shared = uiObject.GetComponent(this);

  Assert(shared != nullptr, "UIComponent failed to get own shared pointer: it was not found in it's uiObject list");

  return RequirePointerCast<UIComponent>(shared);
}
