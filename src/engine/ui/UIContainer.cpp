#include "UIContainer.h"

using namespace std;

UIContainer::UIContainer(Canvas &canvas, string name, int gameSceneId, int id)
    : UIObject(canvas, name, gameSceneId, id) {}

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

  // Execute on this object (bottom up case)
  if (topDown == false)
    callback(*this);
}

std::shared_ptr<GameObject> UIContainer::InternalGetParent() const
{
  if (IsCanvasRoot())
    return RequirePointerCast<GameObject>(canvas.GetShared());

  return GetParent();
}

void UIContainer::RegisterLayer()
{
  if (GetRenderLayer() != RenderLayer::None)
  {
    GetScene()->RegisterLayerRenderer(GetScene()->RequireUIObject<Renderable>(id));
  }
}
