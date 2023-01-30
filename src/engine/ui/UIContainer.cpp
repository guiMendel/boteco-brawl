#include "UIContainer.h"

using namespace std;

UIContainer::UIContainer(Canvas &canvas, string name, int gameSceneId, int id)
    : UIObject(canvas, name, gameSceneId, id) {}

UIContainer::UIContainer(Canvas &canvas, string name, shared_ptr<UIContainer> parent)
    : UIObject(canvas, name, parent) {}

void UIContainer::CascadeDown(function<void(GameObject &)> callback, bool topDown = true) { CascadeDownChildren(callback, topDown); }

std::shared_ptr<GameObject> UIContainer::InternalGetParent() const { return GetParent(); }

void UIContainer::RegisterLayer()
{
  if (GetRenderLayer() != RenderLayer::None)
  {
    GetScene()->RegisterLayerRenderer(GetScene()->RequireUIObject<Renderable>(id));
  }
}
