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

  // Current speed of camera
  Vector2 speed;

  Vector2 GetPosition() const;

  void SetPosition(Vector2 newPosition);

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

  // Convert coordinates
  Vector2 ScreenToWorld(const Vector2 &screenCoordinates) const
  {
    return screenCoordinates + rawPosition;
  }

  // Convert coordinates
  Vector2 WorldToScreen(const Vector2 &worldCoordinates) const
  {
    return worldCoordinates - rawPosition;
  }

  Rectangle ScreenToWorld(const Rectangle &screenCoordinates) const
  {
    return screenCoordinates + rawPosition;
  }
  Rectangle WorldToScreen(const Rectangle &worldCoordinates) const
  {
    return worldCoordinates - rawPosition;
  }

  static Camera &GetInstance()
  {
    static Camera instance;

    return instance;
  }

private:
  // Singleton
  Camera(){};

  // World coordinates of camera's top left corner
  Vector2 rawPosition{-Game::screenWidth / 2.0f, -Game::screenHeight / 2.0f};

  // Which game object to follow
  std::weak_ptr<GameObject> weakFocus;

  // How much time to wait before starting to follow target
  float timeLeftToFollow{0.0f};

  // Move the camera the current speed
  void Move(Vector2 speedModification, float deltaTime);
};

#endif