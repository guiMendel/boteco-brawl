#include <algorithm>
#include <iostream>
#include <memory>
#include "Camera.h"
#include "InputManager.h"
#include "Game.h"

using namespace std;

static const float screenRatio = float(Game::screenWidth) / float(Game::screenHeight);

#define LENGTH(array) sizeof(array) / sizeof(array[0])
#define CLAMP(value, minValue, maxValue) min(max(value, minValue), maxValue)

const Color Camera::defaultBackground{0, 30, 137};
const float Camera::initialSize{5};

static const Vector2 screenQuarter = Vector2(-Game::screenWidth / 2.0f, -Game::screenHeight / 2.0f);

shared_ptr<Camera> Camera::GetMain()
{
  auto cameras = Game::GetInstance().GetScene()->GetCameras();

  Assert(cameras.empty() == false, "Trying to get Main camera when no cameras are registered");

  return cameras.front();
}

Camera::Camera(GameObject &associatedObject) : WorldComponent(associatedObject)
{
  SetSize(initialSize);
}

void Camera::RegisterToScene()
{
  GetScene()->RegisterCamera(RequirePointerCast<Camera>(GetShared()));
}

// Get how many units occupy half the camera's height
float Camera::GetSize() const { return Game::screenHeight * unitsPerRealPixel / 2; }

void Camera::SetSize(float newSize)
{
  realPixelsPerUnit = Game::screenHeight / 2 / newSize;
  unitsPerRealPixel = 1 / realPixelsPerUnit;
}

Vector2 Camera::GetTopLeft() const
{
  return worldObject.GetPosition() + screenQuarter * unitsPerRealPixel;
}

void Camera::SetTopLeft(Vector2 newPosition)
{
  worldObject.SetPosition(newPosition - screenQuarter * unitsPerRealPixel);
}

// Convert coordinates
Vector2 Camera::ScreenToWorld(const Vector2 &screenCoordinates) const
{
  return screenCoordinates * unitsPerRealPixel + GetTopLeft();
}

// Convert coordinates
Vector2 Camera::WorldToScreen(const Vector2 &worldCoordinates) const
{
  return WorldToScreen(worldCoordinates, false);
}

// Convert coordinates & dimensions (screen pixels to game units)
Rectangle Camera::ScreenToWorld(const Rectangle &screenRectangle) const
{
  return screenRectangle * unitsPerRealPixel + GetTopLeft();
}

// Convert coordinates & dimensions (game units to screen pixels)
Rectangle Camera::WorldToScreen(const Rectangle &worldRectangle) const
{
  return (worldRectangle - GetTopLeft()) * realPixelsPerUnit;
}

Rectangle Camera::ToRectangle() const
{
  float doubleSize = GetSize() * 2;

  return Rectangle(worldObject.GetPosition(), doubleSize * screenRatio, doubleSize);
}

Vector2 Camera::WorldToScreen(const Vector2 &worldCoordinates, bool useOriginPosition) const
{
  Vector2 offset = useOriginPosition ? worldObject.GetPosition() : Vector2{0, 0};

  return (worldCoordinates - GetTopLeft() + offset) * realPixelsPerUnit;
}
