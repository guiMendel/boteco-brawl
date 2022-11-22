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
  auto cameras = Game::GetInstance().GetState().GetCameras();

  if (cameras.empty())
  {
    cout << "WARNING: Trying to get Main camera when no cameras are registered" << endl;
    return nullptr;
  }

  return cameras.front();
}

Camera::Camera(GameObject &associatedObject, float size)
    : Component(associatedObject)
{
  SetSize(size);
}

void Camera::Start()
{
  gameState.RegisterCamera(dynamic_pointer_cast<Camera>(GetShared()));
}

// Get how many units occupy half the camera's height
float Camera::GetSize() const { return Game::screenHeight / pixelsPerUnit / 2; }

void Camera::SetSize(float newSize)
{
  pixelsPerUnit = Game::screenHeight / 2 / newSize;
}

Vector2 Camera::GetTopLeft() const
{
  static auto currentPixelsPerUnit = pixelsPerUnit;

  static Vector2 topLeftDisplacement = screenQuarter / currentPixelsPerUnit;

  if (currentPixelsPerUnit != pixelsPerUnit)
  {
    currentPixelsPerUnit = pixelsPerUnit;
    topLeftDisplacement = screenQuarter / currentPixelsPerUnit;
  }

  return gameObject.GetPosition() + topLeftDisplacement;
}

void Camera::SetTopLeft(Vector2 newPosition)
{
  static auto currentPixelsPerUnit = pixelsPerUnit;

  static Vector2 topLeftDisplacement = screenQuarter / currentPixelsPerUnit;

  if (currentPixelsPerUnit != pixelsPerUnit)
  {
    currentPixelsPerUnit = pixelsPerUnit;
    topLeftDisplacement = screenQuarter / currentPixelsPerUnit;
  }

  gameObject.SetPosition(newPosition - topLeftDisplacement);
}

// Convert coordinates
Vector2 Camera::ScreenToWorld(const Vector2 &screenCoordinates) const
{
  return screenCoordinates / pixelsPerUnit + GetTopLeft();
}

// Convert coordinates
Vector2 Camera::WorldToScreen(const Vector2 &worldCoordinates) const
{
  return (worldCoordinates - GetTopLeft()) * pixelsPerUnit;
}

Rectangle Camera::ScreenToWorld(const Rectangle &screenCoordinates) const
{
  return screenCoordinates / pixelsPerUnit + GetTopLeft();
}

Rectangle Camera::WorldToScreen(const Rectangle &worldCoordinates) const
{
  return (worldCoordinates - GetTopLeft()) * pixelsPerUnit;
}
