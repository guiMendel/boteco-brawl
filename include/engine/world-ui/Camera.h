#ifndef __CAMERA__
#define __CAMERA__

#include "Game.h"
#include "WorldObject.h"
#include "Vector2.h"
#include "Rectangle.h"
#include "WorldComponent.h"
#include <memory>

class SpriteRenderer;

class Camera : public WorldComponent
{
  friend class SpriteRenderer;
  
public:
  static std::shared_ptr<Camera> GetMain();

  static const float initialSize;

  Camera(GameObject &associatedObject);

  virtual ~Camera() {}

  Vector2 GetPosition() const { return worldObject.GetPosition(); }

  void SetPosition(Vector2 newPosition) { worldObject.SetPosition(newPosition); }

  // Get how many units occupy half the camera's height
  float GetSize() const;

  void SetSize(float newSize);

  Vector2 GetTopLeft() const;

  void SetTopLeft(Vector2 newPosition);

  void Reset()
  {
    SetPosition(Vector2::Zero());
  }

  void RegisterToScene() override;

  // Convert coordinates (screen pixels to game units)
  Vector2 ScreenToWorld(const Vector2 &screenCoordinates) const;

  // Convert coordinates (game units to screen pixels)
  Vector2 WorldToScreen(const Vector2 &worldCoordinates) const;

  // Convert coordinates & dimensions (screen pixels to game units)
  Rectangle ScreenToWorld(const Rectangle &screenRectangle) const;

  // Convert coordinates & dimensions (game units to screen pixels)
  Rectangle WorldToScreen(const Rectangle &worldRectangle) const;

  float GetRealPixelsPerUnit() const { return realPixelsPerUnit; }
  float GetUnitsPerRealPixel() const { return unitsPerRealPixel; }

  // Generates a rectangle with the camera's global position and dimensions in game units
  Rectangle ToRectangle() const;

  // Color of camera's background
  Color background{defaultBackground};

  // Default camera background
  static const Color defaultBackground;

private:
  // Allows to "pretend" camera's position is set on origin
  Vector2 WorldToScreen(const Vector2 &worldCoordinates, bool useOriginPosition) const;

  // Current unit resolution of camera
  float realPixelsPerUnit;

  // Used in math operations
  float unitsPerRealPixel;
};

#endif