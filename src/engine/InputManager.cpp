#include "InputManager.h"
#include "Camera.h"
#include <SDL.h>

#define CONTROLLER_AXIS_MAX 32767.0f

using namespace std;

const int InputManager::joystickDeadZone{8000};

// Flattens joystick axis value in range -1 to 1, and sets it to 0 if below deadzone
float InputManager::TreatAxisValue(int value)
{
  float flattenValue = float(value) / CONTROLLER_AXIS_MAX;
  return abs(flattenValue) < joystickDeadZone ? 0 : flattenValue;
}

void InputManager::Update()
{
  SDL_Event event;

  // Get mouse coords
  SDL_GetMouseState(&mouseX, &mouseY);

  // Handle changes to connected controllers
  // ConnectControllers();

  // Reset quit request
  quitRequested = false;

  // Increment counter
  updateCounter++;

  // Game controller mapping for analogs in last frame
  static unordered_map<int, Vector2> lastLeftControllerAnalogs, lastRightControllerAnalogs;

  // Game controller mapping for analogs in this frame
  static unordered_map<int, Vector2> currentLeftControllerAnalogs, currentRightControllerAnalogs;

  // If there are any input events in the SDL stack pile, this function returns 1 and sets the argument to next event
  while (SDL_PollEvent(&event))
  {
    // Quit on quit event
    if (event.type == SDL_QUIT)
      quitRequested = true;

    // ===  MOUSE

    // On click event
    else if (event.type == SDL_MOUSEBUTTONDOWN)
    {
      auto button = event.button.button;

      mouseState[button] = true;
      mouseUpdate[button] = updateCounter;
    }

    // On un-click event
    else if (event.type == SDL_MOUSEBUTTONUP)
    {
      auto button = event.button.button;

      mouseState[button] = false;
      mouseUpdate[button] = updateCounter;
    }

    // ===  KEYBOARD

    // On keyboard press
    else if (event.type == SDL_KEYDOWN)
    {
      // Ignore repetitions
      if (!event.key.repeat)
      {
        auto symbol = event.key.keysym.sym;

        keyState[symbol] = true;
        keyUpdate[symbol] = updateCounter;
      }
    }

    // On keyboard release
    else if (event.type == SDL_KEYUP)
    {
      auto symbol = event.key.keysym.sym;

      keyState[symbol] = false;
      keyUpdate[symbol] = updateCounter;
    }

    // ===  CONTROLLERS

    // On controller connected
    else if (event.type == SDL_CONTROLLERDEVICEADDED)
    {
      cout << "Controller connected. Device index: " << event.cdevice.which << endl;

      OpenController(event.cdevice.which);
    }

    // On controller disconnected
    else if (event.type == SDL_CONTROLLERDEVICEREMOVED)
    {
      cout << "Controller disconnected. Instance id: " << event.cdevice.which << endl;

      // Remove this entry
      controllers.erase(event.cdevice.which);
    }

    // On controller remapped
    else if (event.type == SDL_CONTROLLERDEVICEREMAPPED)
      cout << "Controller remapped. Instance id: " << event.cdevice.which << endl;

    // On controller button press
    else if (event.type == SDL_CONTROLLERBUTTONDOWN)
      OnControllerButtonPress.Invoke((SDL_GameControllerButton)event.cbutton.button, event.cbutton.which);

    // On controller button release
    else if (event.type == SDL_CONTROLLERBUTTONUP)
      OnControllerButtonRelease.Invoke((SDL_GameControllerButton)event.cbutton.button, event.cbutton.which);

    // On controller axis move
    else if (event.type == SDL_CONTROLLERAXISMOTION)
    {
      float value = TreatAxisValue(event.caxis.value);

      // Register this axis variation for this frame
      if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
        currentLeftControllerAnalogs[event.caxis.which].x = value;

      if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
        currentLeftControllerAnalogs[event.caxis.which].y = value;

      if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
        currentRightControllerAnalogs[event.caxis.which].x = value;

      if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
        currentRightControllerAnalogs[event.caxis.which].y = value;
    }
  }

  // Detect variation of controller analog
  for (auto [instanceId, analog] : currentLeftControllerAnalogs)
  {
    // Compare to the analog state last frame
    if (analog != lastLeftControllerAnalogs[instanceId])
      // If they are different, raise
      OnControllerLeftAnalog.Invoke(analog, instanceId);
  }
  for (auto [instanceId, analog] : currentRightControllerAnalogs)
  {
    // Compare to the analog state last frame
    if (analog != lastRightControllerAnalogs[instanceId])
      // If they are different, raise
      OnControllerRightAnalog.Invoke(analog, instanceId);
  }

  // Store for next frame
  lastLeftControllerAnalogs = currentLeftControllerAnalogs;
  lastRightControllerAnalogs = currentRightControllerAnalogs;

  // Empty these for next frame
  currentLeftControllerAnalogs.clear();
  currentRightControllerAnalogs.clear();
}

void InputManager::OpenController(int index)
{
  // Create device
  auto controller = make_shared<ControllerDevice>(index);

  // Register it
  controllers[controller->GetId()] = controller;

  // Search for available players
  controller->SearchPlayersForAssociation();
}

Vector2 InputManager::GetMouseWorldCoordinates() const
{
  return Camera::GetMain()->ScreenToWorld(Vector2((float)mouseX, (float)mouseY));
}

std::shared_ptr<ControllerDevice> InputManager::GetController(int id)
{
  if (controllers.count(id) == 0)
    return nullptr;

  return controllers[id];
}
