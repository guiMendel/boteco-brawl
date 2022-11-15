#include <algorithm>
#include <iostream>
#include "Camera.h"
#include "InputManager.h"
#include "Game.h"

using namespace std;

#define LENGTH(array) sizeof(array) / sizeof(array[0])
#define CLAMP(value, minValue, maxValue) min(max(value, minValue), maxValue)

// Acceleration applied each frame to the camera towards 0 speed
const int Camera::gravity{70};

// Max speed for the camera, in pixels per second
const int Camera::maxSpeed{700};

// Acceleration applied to the camera on user input
const int Camera::acceleration{4000};

// How many seconds the camera waits before starting to follow the target
const float Camera::followDelay{2.0f};

// How far from the target camera can be before starting to follow
const float Camera::maxFocusDistance{1.0f};

Vector2 GetInputSpeedChange();

Vector2 GetGravitySpeedChange(Vector2 speed, float gravity);

Vector2 Camera::GetPosition() const
{
  return rawPosition + Vector2(Game::screenWidth / 2, Game::screenHeight / 2);
}

void Camera::SetPosition(Vector2 newPosition)
{
  rawPosition = newPosition - Vector2(Game::screenWidth / 2, Game::screenHeight / 2);
}

void Camera::Update(float deltaTime)
{
  // Get displacement from input, taking delta time in consideration
  auto frameSpeedChange = GetInputSpeedChange() * acceleration * deltaTime;

  // If there was input, reset follow delay (convert seconds to milliseconds)
  if (frameSpeedChange)
    timeLeftToFollow = followDelay;

  // If no input
  else
  {

    // If follow delay is over OR there is no target
    if (weakFocus.expired() == false && timeLeftToFollow <= 0.0f)
    {
      LOCK(weakFocus, focus);

      frameSpeedChange = Vector2::Zero();
      SetPosition(focus->GetPosition());
    }

    // If timer is not up yet
    else
    {
      // By default, apply gravity
      if (speed)
        frameSpeedChange = GetGravitySpeedChange(speed, gravity);

      // Discount it
      if (timeLeftToFollow > 0.0f)
        timeLeftToFollow -= deltaTime;
    }
  }

  // Move
  Move(frameSpeedChange, deltaTime);
}

void Camera::Move(Vector2 speedModification, float deltaTime)
{
  // Add to the camera speed
  speed += speedModification;

  // Check that it does not exceed the limits
  speed.CapMagnitude(maxSpeed);

  // Displace it
  rawPosition += speed * deltaTime;
}

// Gets the displacement directions from input
Vector2 GetInputSpeedChange()
{
  // Get input reference
  auto inputManager = InputManager::GetInstance();

  // Will hold camera displacement values
  Vector2 frameSpeedChange{Vector2::Zero()};

  // Map each key direction to an index
  typedef decltype(UP_ARROW_KEY) KeyType;

  KeyType keyDirectionMap[] =
      {UP_ARROW_KEY, LEFT_ARROW_KEY, DOWN_ARROW_KEY, RIGHT_ARROW_KEY};

  // Map each displacement direction to an index, corresponding to the key direction map
  Vector2 displacementMap[LENGTH(keyDirectionMap)] =
      {Vector2::Down(), Vector2::Left(), Vector2::Up(), Vector2::Right()};

  // Catch button presses
  for (size_t i = 0; i < LENGTH(keyDirectionMap); ++i)
  {
    // Check if this direction is pressed
    if (inputManager.IsKeyDown(keyDirectionMap[i]))
      // If so, add this displacement to the sum
      frameSpeedChange += displacementMap[i];
  }

  return frameSpeedChange;
}
Vector2 GetGravitySpeedChange(Vector2 speed, float gravity)
{
  // Set the displacement to the opposite direction of current speed
  Vector2 frameSpeedChange = Vector2(speed).Normalized() * -gravity;

  // Cap to the current speed value
  frameSpeedChange.CapMagnitude(speed.Magnitude());

  return frameSpeedChange;
}