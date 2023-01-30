#include "UIObject.h"
#include "UIContainer.h"
#include "Canvas.h"

using namespace std;

UIObject::UIObject(Canvas &canvas, string name, int gameSceneId, int id = -1)
    : GameObject(name, gameSceneId, id),
      width(UIDimension::Horizontal, GetShared()),
      height(UIDimension::Vertical, GetShared()),
      padding(GetShared()),
      margin(GetShared()),
      canvas(canvas) {}

UIObject::UIObject(Canvas &canvas, string name, shared_ptr<UIContainer> parent = nullptr)
    : GameObject(name),
      width(UIDimension::Horizontal, GetShared()),
      height(UIDimension::Vertical, GetShared()),
      padding(GetShared()),
      margin(GetShared()),
      canvas(canvas) {}

UIObject::~UIObject() {}

Vector2 UIObject::GetPosition() { return updatedPosition; }

UIDimension &UIObject::GetSize(UIDimension::Axis axis) { return axis == UIDimension::Horizontal ? width : height; }

bool UIObject::IsCanvasRoot() const { return canvas.root.id == id; }

shared_ptr<UIContainer> UIObject::GetParent() const
{
  if (IsCanvasRoot())
    return nullptr;

  return Lock(weakParent);
}

void UIObject::SetParent(shared_ptr<UIContainer> newParent) { weakParent = newParent; }

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
