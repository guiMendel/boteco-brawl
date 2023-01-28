#include <algorithm>
#include <iostream>
#include <memory>
#include "Camera.h"
#include "InputManager.h"
#include "Game.h"

using namespace std;

#define LENGTH(array) sizeof(array) / sizeof(array[0])
#define CLAMP(value, minValue, maxValue) min(max(value, minValue), maxValue)

Vector2 screenQuarter = Vector2(-Game::screenWidth / 2.0f, -Game::screenHeight / 2.0f);

shared_ptr<Camera> Camera::GetMain()
{
  auto cameras = Game::GetInstance().GetState()->GetCameras();

  Assert(cameras.empty() == false, "Trying to get Main camera when no cameras are registered");

  return cameras.front();
}

Camera::Camera(WorldObject &associatedObject, float size)
    : Component(associatedObject)
{
  SetSize(size);
}

void Camera::RegisterToState()
{
  GetState()->RegisterCamera(dynamic_pointer_cast<Camera>(GetShared()));
}

// Get how many units occupy half the camera's height
float Camera::GetSize() const { return Game::screenHeight / realPixelsPerUnit / 2; }

void Camera::SetSize(float newSize)
{
  realPixelsPerUnit = Game::screenHeight / 2 / newSize;
}

Vector2 Camera::GetTopLeft() const
{
  static auto currentPixelsPerUnit = realPixelsPerUnit;

  static Vector2 topLeftDisplacement = screenQuarter / currentPixelsPerUnit;

  if (currentPixelsPerUnit != realPixelsPerUnit)
  {
    currentPixelsPerUnit = realPixelsPerUnit;
    topLeftDisplacement = screenQuarter / currentPixelsPerUnit;
  }

  return worldObject.GetPosition() + topLeftDisplacement;
}

void Camera::SetTopLeft(Vector2 newPosition)
{
  static auto currentPixelsPerUnit = realPixelsPerUnit;

  static Vector2 topLeftDisplacement = screenQuarter / currentPixelsPerUnit;

  if (currentPixelsPerUnit != realPixelsPerUnit)
  {
    currentPixelsPerUnit = realPixelsPerUnit;
    topLeftDisplacement = screenQuarter / currentPixelsPerUnit;
  }

  worldObject.SetPosition(newPosition - topLeftDisplacement);
}

// Convert coordinates
Vector2 Camera::ScreenToWorld(const Vector2 &screenCoordinates) const
{
  return screenCoordinates / realPixelsPerUnit + GetTopLeft();
}

// Convert coordinates
Vector2 Camera::WorldToScreen(const Vector2 &worldCoordinates) const
{
  return (worldCoordinates - GetTopLeft()) * realPixelsPerUnit;
}

// Convert coordinates & dimensions (screen pixels to game units)
Rectangle Camera::ScreenToWorld(const Rectangle &screenRectangle) const
{
  return screenRectangle / realPixelsPerUnit + GetTopLeft();
}

// Convert coordinates & dimensions (game units to screen pixels)
Rectangle Camera::WorldToScreen(const Rectangle &worldRectangle) const
{
  return (worldRectangle - GetTopLeft()) * realPixelsPerUnit;
}

Rectangle Camera::ToRectangle() const
{
  static const float screenRatio = Game::screenWidth / Game::screenHeight;

  float doubleSize = GetSize() * 2;

  return Rectangle(worldObject.GetPosition(), doubleSize * screenRatio, doubleSize);
}
