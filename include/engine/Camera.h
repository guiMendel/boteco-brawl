#ifndef __CAMERA__
#define __CAMERA__

#include "Game.h"
#include "GameObject.h"
#include "Vector2.h"
#include "Rectangle.h"
#include "Component.h"
#include <memory>

class Camera : public Component
{
public:
  static std::shared_ptr<Camera> GetMain();

  Camera(GameObject &associatedObject, float size = 5);

  virtual ~Camera() {}

  Vector2 GetPosition() const { return gameObject.GetPosition(); }

  void SetPosition(Vector2 newPosition) { gameObject.SetPosition(newPosition); }

  // Get how many units occupy half the camera's height
  float GetSize() const;

  void SetSize(float newSize);

  Vector2 GetTopLeft() const;

  void SetTopLeft(Vector2 newPosition);

  void Reset()
  {
    SetPosition(Vector2::Zero());
  }

  void RegisterToState() override;

  // Convert coordinates (screen pixels to game units)
  Vector2 ScreenToWorld(const Vector2 &screenCoordinates) const;

  // Convert coordinates (game units to screen pixels)
  Vector2 WorldToScreen(const Vector2 &worldCoordinates) const;

  // Convert coordinates & dimensions (screen pixels to game units)
  Rectangle ScreenToWorld(const Rectangle &screenRectangle) const;

  // Convert coordinates & dimensions (game units to screen pixels)
  Rectangle WorldToScreen(const Rectangle &worldRectangle) const;

  float GetRealPixelsPerUnit() const { return realPixelsPerUnit; }

private:
  // Current unit resolution of camera
  float realPixelsPerUnit;
};

#endif