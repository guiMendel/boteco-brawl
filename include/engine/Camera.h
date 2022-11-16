#ifndef __CAMERA__
#define __CAMERA__

#include "Game.h"
#include "GameObject.h"
#include "Vector2.h"
#include "Rectangle.h"
#include <memory>

class Camera
{
public:
  // Acceleration applied each frame to the camera towards 0 speed
  static const int gravity;

  // Max speed for the camera, in pixels per second
  static const int maxSpeed;

  // Acceleration applied to the camera on user input
  static const int acceleration;

  // How many seconds the camera waits before starting to follow the target
  static const float followDelay;

  // How far from the target camera can be before starting to follow
  static const float maxFocusDistance;

  Vector2 GetPosition() const;

  void SetPosition(Vector2 newPosition);

  // Get how many units occupy half the camera's height
  float GetSize() const;

  void SetSize(float newSize);

  Vector2 GetRawPosition() const { return rawPosition; }

  void SetRawPosition(Vector2 newPosition) { rawPosition = newPosition; }

  void Reset()
  {
    SetPosition(Vector2::Zero());
    speed = Vector2::Zero();
    weakFocus.reset();
  }

  // Start following new object
  void Follow(std::shared_ptr<GameObject> newFocus) { weakFocus = newFocus; }

  // Follow no object
  void Unfollow() { weakFocus.reset(); }

  // Update frame
  void Update(float deltaTime);

  // Convert coordinates (screen pixels to game units)
  Vector2 ScreenToWorld(const Vector2 &screenCoordinates) const;

  // Convert coordinates (game units to screen pixels)
  Vector2 WorldToScreen(const Vector2 &worldCoordinates) const;

  // Convert coordinates & dimensions (screen pixels to game units)
  Rectangle ScreenToWorld(const Rectangle &screenCoordinates) const;

  // Convert coordinates & dimensions (game units to screen pixels)
  Rectangle WorldToScreen(const Rectangle &worldCoordinates) const;

  float GetPixelsPerUnit() const { return pixelsPerUnit; }

  static Camera &GetInstance()
  {
    static Camera instance;

    return instance;
  }

  // Current speed of camera
  Vector2 speed;

private:
  // Singleton
  Camera();

  // Move the camera the current speed
  void Move(Vector2 speedModification, float deltaTime);

  // Current unit resolution of camera
  float pixelsPerUnit;

  // World coordinates of camera's top left corner
  Vector2 rawPosition;

  // Which game object to follow
  std::weak_ptr<GameObject> weakFocus;

  // How much time to wait before starting to follow target
  float timeLeftToFollow{0.0f};
};

#endif