#include "Canvas.h"
#include "Debug.h"

using namespace std;

// TODO fix this
Canvas::Canvas(GameObject &associatedObject, Space space, Vector2 size, shared_ptr<Camera> camera)
    : WorldComponent(associatedObject),
      root(GetScene()->NewObject<UIContainer>(*this, "UIRoot", nullptr)),
      weakCamera(camera)
{
  // Initialize root
  InitializeRootStyle();
  SetSpace(space);
  SetSize(size);
  root->InitializeDimensions();
}

Canvas::~Canvas()
{
}

Vector2 Canvas::GetAnchorPosition() const
{
  switch (space)
  {
  case Space::Global:
    return Camera::GetMain()->ScreenToWorld({0, 0});

  case Space::WorldFixedSize:
    return gameObject.GetPosition() - Camera::GetMain()->GetUnitsPerRealPixel() * GetSize() * anchorPoint;

  case Space::World:
    return gameObject.GetPosition() - GetSize() * anchorPoint;
  }

  throw runtime_error("Unrecognized canvas space");
}

Vector2 Canvas::CanvasToWorld(Vector2 position, shared_ptr<Camera> camera) const
{
  return GetAnchorPosition() + position * camera->GetUnitsPerRealPixel();
}

Vector2 Canvas::WorldToCanvas(Vector2 position, shared_ptr<Camera> camera) const
{
  return (position - GetAnchorPosition()) * camera->GetRealPixelsPerUnit();
}

Vector2 Canvas::CanvasToScreen(Vector2 position, shared_ptr<Camera> camera) const
{
  return camera->WorldToScreen(GetAnchorPosition()) + position;
}

Vector2 Canvas::ScreenToCanvas(Vector2 position, shared_ptr<Camera> camera) const
{
  return position - camera->WorldToScreen(GetAnchorPosition());
}

void Canvas::CascadeDown(function<void(GameObject &)> callback, bool topDown)
{
  // Carry cascading on down the canvas object tree
  root->CascadeDown(callback, topDown);
}

void Canvas::InitializeRootStyle()
{
  root->style->fontPath.Set("./assets/engine/fonts/PixelOperator.ttf");
  root->style->fontSize.Set(20);
  root->style->imageColor.Set(Color::White());
  root->style->imageScaling.Set(1);
  root->style->renderLayer.Set(RenderLayer::UI);
  root->style->renderOrder.Set(0);
  root->style->textBorderColor.Set(Color::Black());
  root->style->textBorderSize.Set(0);
  root->style->textColor.Set(Color::White());
}

void Canvas::Render()
{
  // auto camera = Camera::GetMain();

  // Vector2 drawSize;

  // if (space == Space::World)
  //   drawSize = Vector2{float(Game::screenWidth), float(Game::screenHeight)} * camera->GetUnitsPerRealPixel();
  // else if (space == Space::WorldFixedSize)
  //   drawSize = GetSize() * camera->GetUnitsPerRealPixel();
  // else
  //   drawSize = GetSize();

  // Vector2 position;

  // if (space == Space::Global)
  //   position = camera->ScreenToWorld({Game::screenWidth / 2.0f, Game::screenHeight / 2.0f});
  // else
  // {
  //   Vector2 anchorDisplacement{
  //       Lerp(-drawSize.x / 2, drawSize.x / 2, anchorPoint.x),
  //       Lerp(-drawSize.y / 2, drawSize.y / 2, anchorPoint.y)};

  //   position = gameObject.GetPosition() - anchorDisplacement;
  // }

  // auto color = Color::Cyan();
  // color.alpha = 70;

  // Debug::DrawBox(Rectangle(position, drawSize.x, drawSize.y), color);
}

void Canvas::OnBeforeDestroy()
{
  // Destroy canvas object tree
  root->InternalDestroy();
}

Vector2 Canvas::GetSize() const { return {root->width.value, root->height.value}; }

void Canvas::SetSize(Vector2 newSize)
{
  auto oldSize = GetSize();

  // Check space
  if (space == Space::WorldFixedSize)
  {
    root->width.Set(UIDimension::RealPixels, newSize.x);
    root->height.Set(UIDimension::RealPixels, newSize.y);
  }
  else if (space == Space::World)
  {
    root->width.Set(UIDimension::WorldUnits, newSize.x);
    root->height.Set(UIDimension::WorldUnits, newSize.y);
  }
  // Global space ignore this
  else if (space == Space::Global)
    return;

  if (newSize != oldSize)
    OnChangeSize.Invoke(newSize, oldSize);
}

Canvas::Space Canvas::GetSpace() const
{
  return space;
}

void Canvas::SetSpace(Space newSpace)
{
  auto newSize = GetSize();

  space = newSpace;

  // Catch global space
  if (newSpace == Space::Global)
  {
    root->width.Set(UIDimension::RealPixels, Game::screenWidth);
    root->height.Set(UIDimension::RealPixels, Game::screenHeight);

    return;
  }

  SetSize(newSize);
}
