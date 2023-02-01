#include "UIObject.h"
#include "UIContainer.h"
#include "Canvas.h"

using namespace std;

UIObject::UIObject(Canvas &canvas, string name)
    : GameObject(name),
      width(UIDimension::Horizontal, GetShared()),
      height(UIDimension::Vertical, GetShared()),
      padding(GetShared()),
      margin(GetShared()),
      style(make_unique<UIInheritable>(*this)),
      canvas(canvas) {}

UIObject::UIObject(Canvas &canvas, shared_ptr<UIContainer> parent, string name)
    : UIObject(canvas, name)
{
  SetParent(parent);
}

UIObject::~UIObject() {}

Vector2 UIObject::GetPosition() { return updatedPosition; }

UIDimension &UIObject::GetSize(UIDimension::Axis axis) { return axis == UIDimension::Horizontal ? width : height; }

bool UIObject::IsCanvasRoot() const { return canvas.root != nullptr && canvas.root->id == id; }

shared_ptr<UIContainer> UIObject::GetParent() const
{
  if (IsCanvasRoot())
    return nullptr;

  return Lock(weakParent);
}

void UIObject::SetParent(shared_ptr<UIContainer> newParent)
{
  Assert(IsCanvasRoot() == false, "Canvas root can't have its parent set");

  // If no parent, add canvas root as parent (unless this is the first object to be added)
  if (newParent == nullptr && canvas.root != nullptr)
    newParent = canvas.root;

  // If had a parent before
  if (weakParent.expired() == false)
    UnlinkParent();

  // Add reference to parent
  weakParent = newParent;

  // If not canvas root
  if (IsCanvasRoot() == false)
    // Give parent a reference to self
    newParent->children[id] = weak_ptr(GetShared());
}

bool UIObject::IsDescendantOf(shared_ptr<UIContainer> other) const
{
  if (id == other->id)
    return true;

  if (GetParent() == nullptr)
    return false;

  return GetParent()->IsDescendantOf(other);
}

bool UIObject::SameLineage(shared_ptr<UIObject> first, shared_ptr<UIObject> second)
{
  if (auto firstContainer = dynamic_pointer_cast<UIContainer>(first);
      firstContainer != nullptr && second->IsDescendantOf(firstContainer))
    return true;

  if (auto secondContainer = dynamic_pointer_cast<UIContainer>(second);
      secondContainer != nullptr && first->IsDescendantOf(secondContainer))
    return true;

  return false;
}

std::shared_ptr<UIObject> UIObject::GetShared() { return GetScene()->RequireUIObject<UIObject>(id); }

void UIObject::UnlinkParent()
{
  Assert(IsCanvasRoot() == false, "Canvas root has no parent to unlink");

  // Get parent
  auto parent = GetParent();

  // Get own iterator
  auto iterator = parent->children.find(id);

  // Remove it
  parent->children.erase(iterator);
  weakParent.reset();
}

RenderLayer UIObject::GetRenderLayer()
{
  return style->renderLayer.Get();
}

// Use value from style
int UIObject::GetRenderOrder() { return style->renderOrder.Get(); }

void UIObject::RegisterLayer()
{
  if (GetRenderLayer() != RenderLayer::None)
    GetScene()->RegisterLayerRenderer(GetScene()->RequireUIObject<Renderable>(id));
}

void UIObject::RegisterToScene()
{
  auto currentScene = GetScene()->id;

  if (lastSceneRegisteredTo == currentScene)
    return;

  // Call super's
  GameObject::RegisterToScene();

  // Register self
  RegisterLayer();
}
