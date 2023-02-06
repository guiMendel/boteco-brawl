#include "UIObject.h"
#include "Debug.h"
#include "UIContainer.h"
#include "Canvas.h"

using namespace std;

UIObject::UIObject(Canvas &canvas, string name, std::shared_ptr<UIContainer> parent)
    : GameObject(name),
      width(UIDimension::Horizontal),
      height(UIDimension::Vertical),
      style(make_unique<UIInheritable>(*this)),
      canvas(canvas)
{
  // Subscribe to own dimension change
  auto alertParent = [this](size_t, size_t)
  {
    LOCK(weakParent, parent);

    parent->forceRecalculation = true;
  };

  width.OnRealPixelSizeChange.AddListener("alert-UIContainer-parent", alertParent);
  height.OnRealPixelSizeChange.AddListener("alert-UIContainer-parent", alertParent);
  margin.OnRealPixelSizeChange.AddListener("alert-UIContainer-parent", [alertParent]()
                                           { alertParent(0, 0); });
  //  Padding size change is already included in width and height

  if (IsCanvasRoot() == false)
  {
    // Treat no parent as child of canvas root
    if (parent == nullptr)
      parent = canvas.root;

    // Register parent
    weakParent = parent;
  }
}

UIObject::~UIObject() {}

Vector2 UIObject::GetPosition()
{
  if (IsCanvasRoot())
    return localPosition;

  return Lock(weakParent)->GetContentPosition() + localPosition;
}

Vector2 UIObject::GetContentPosition()
{
  return GetPosition() + Vector2{float(padding.left.AsRealPixels()), float(padding.top.AsRealPixels())};
}

UIDimension &UIObject::GetDimension(UIDimension::Axis axis)
{
  return axis == UIDimension::Horizontal ? width : height;
}

// If there is no root, then this object must be the root under construction
bool UIObject::IsCanvasRoot() const { return (canvas.root == nullptr) || (canvas.root->id == id); }

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
  {
    // Get arrange order for this parent
    arrangeOrder = newParent->arrangeOrderGenerator++;

    // Give parent a reference to self
    newParent->children[id] = GetShared();
  }
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

std::shared_ptr<UIObject> UIObject::GetShared()
{
  return GetScene()->RequireUIObject<UIObject>(id);
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
  {
    GetScene()->RegisterLayerRenderer(GetScene()->RequireUIObject<Renderable>(id));
  }
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

void UIObject::Render()
{
  // Don't render root
  if (IsCanvasRoot())
    return;

  auto camera = Camera::GetMain();

  auto color = Color::Pink();
  color.alpha = 170;

  Debug::DrawBox(Rectangle(
                     Rectangle::TopLeftInitialize,
                     canvas.CanvasToWorld(GetPosition()),
                     GetPaddedWidth() * camera->GetUnitsPerRealPixel(),
                     GetPaddedHeight() * camera->GetUnitsPerRealPixel()),
                 color);
}

auto UIObject::DestroySelf() -> std::unordered_map<int, std::weak_ptr<UIObject>>::iterator
{
  // cout << "Destroying " << *this << endl;

  // Store pointer to self for later
  auto shared = GetShared();

  // Call base destroy method to clean up components & remove reference from scene
  GameObject::InternalDestroy();

  // Remove this object's reference from it's parent
  unordered_map<int, weak_ptr<UIObject>>::iterator iterator;

  if (IsCanvasRoot())
    canvas.root.reset();
  else if (weakParent.expired() == false)
    iterator = UnlinkParent();

  // Ensure no more references to self than the one in this function and the one which called this function
  Assert(shared.use_count() <= 2, "Found " + to_string(shared.use_count() - 2) + " leaked references to world object " + GetName() + " when trying to destroy it");

  return iterator;
}

auto UIObject::UnlinkParent() -> std::unordered_map<int, std::weak_ptr<UIObject>>::iterator
{
  Assert(IsCanvasRoot() == false, "Canvas root has no parent to unlink");

  // Get parent
  auto parent = GetParent();

  // Warn it to recalculate children box
  parent->forceRecalculation = true;

  // Get own iterator
  auto iterator = parent->children.find(id);

  // Remove it
  if (iterator != parent->children.end())
    iterator = parent->children.erase(iterator);
  weakParent.reset();

  return iterator;
}

void UIObject::InternalDestroy() { DestroySelf(); }

void UIObject::InternalSetParent(std::shared_ptr<GameObject> newParent)
{
  // Ignore this if canvas root
  if (IsCanvasRoot())
    return;

  SetParent(RequirePointerCast<UIContainer>(newParent));
}

void UIObject::InitializeDimensions()
{
  width.SetOwner(GetShared());
  height.SetOwner(GetShared());
  padding.SetOwner(GetShared());
  margin.SetOwner(GetShared());
}

size_t UIObject::GetRealPixelsAlong(UIDimension::Axis axis, bool includePadding, bool includeMargin)
{
  size_t bonus{0};

  // Add padding
  if (includePadding)
    bonus = padding.SumAlong(axis);

  // Add margin
  if (includeMargin)
    bonus += margin.SumAlong(axis);

  return GetDimension(axis).AsRealPixels() + bonus;
}

void UIObject::SetLocalPositionAlong(UIDimension::Axis axis, int mainSize, int crossSize)
{
  if (axis == UIDimension::Horizontal)
  {
    localPosition.x = float(mainSize);
    localPosition.y = float(crossSize);
  }
  else
  {
    localPosition.y = float(mainSize);
    localPosition.x = float(crossSize);
  }
}

void UIObject::PrecalculateDimensions()
{
  width.PrecalculateDefault();
  height.PrecalculateDefault();
  margin.PrecalculateDefault();
  padding.PrecalculateDefault();
}

size_t UIObject::GetPaddedWidth() { return width.AsRealPixels() + padding.SumAlong(UIDimension::Horizontal); }

size_t UIObject::GetPaddedHeight() { return height.AsRealPixels() + padding.SumAlong(UIDimension::Vertical); }
