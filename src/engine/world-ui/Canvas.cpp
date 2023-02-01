#include "Canvas.h"

using namespace std;

Canvas::Canvas(GameObject &associatedObject, Space space, Vector2 size)
    : WorldComponent(associatedObject),
      space(space),
      size(size),
      root(make_shared<UIContainer>(*this, "UIRoot"))
{
  InitializeRootStyle();
}

Vector2 Canvas::GetTopLeft() const
{
  switch (space)
  {
  case Space::Global:
    return Camera::GetMain()->ScreenToWorld({0, 0});

  case Space::WorldFixedSize:
    return gameObject.GetPosition() - Camera::GetMain()->GetUnitsPerRealPixel() * size / 2;

  case Space::World:
    return gameObject.GetPosition() - size / 2;
  }

  throw runtime_error("ERROR: Unrecognized canvas space");
}

Vector2 Canvas::CanvasToWorld(Vector2 position, shared_ptr<Camera> camera) const
{
  return GetTopLeft() + position * camera->GetUnitsPerRealPixel();
}

Vector2 Canvas::WorldToCanvas(Vector2 position, shared_ptr<Camera> camera) const
{
  return (position - GetTopLeft()) * camera->GetRealPixelsPerUnit();
}

Vector2 Canvas::CanvasToScreen(Vector2 position, shared_ptr<Camera> camera) const
{
  return camera->WorldToScreen(GetTopLeft()) + position;
}

Vector2 Canvas::ScreenToCanvas(Vector2 position, shared_ptr<Camera> camera) const
{
  return position - camera->WorldToScreen(GetTopLeft());
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
