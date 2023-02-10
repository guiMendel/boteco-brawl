#include "InputManager.h"
#include "Camera.h"
#include <SDL.h>

#define CONTROLLER_AXIS_MAX 32767.0f

using namespace std;

const float InputManager::joystickDeadZone{0.1f};

// Maximum seconds an event poll can take before discounting it's duration from deltaTime
const float maximumPollDuration{0.01f};

// Wraps SDL_PollEvent to also measure how long it took to execute
// Sums execution duration to the timer variable
bool PollEventsTimed(SDL_Event *event, float &timer);

// Flattens joystick axis value in range -1 to 1, and sets it to 0 if below deadzone
float InputManager::TreatAxisValue(int value)
{
  float flattenValue = float(value) / CONTROLLER_AXIS_MAX;
  return abs(flattenValue) < joystickDeadZone ? 0 : flattenValue;
}

float InputManager::Update()
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

  // How long SDL_PollEvent will take to execute, in total
  float eventPollingDelay{0};

  // If there are any input events in the SDL stack pile, this function returns 1 and sets the argument to next event
  while (PollEventsTimed(&event, eventPollingDelay))
  {
    // Quit on quit event
    if (event.type == SDL_QUIT)
    {
      MESSAGE << "Quit" << endl;
      quitRequested = true;
    }

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
      auto symbol = event.key.keysym.sym;

      // Ignore repetitions
      if (!event.key.repeat)
      {
        keyState[symbol] = true;
        keyUpdate[symbol] = updateCounter;

        OnKeyPress.Invoke(symbol);
      }

      OnKeyDown.Invoke(symbol);
    }

    // On keyboard release
    else if (event.type == SDL_KEYUP)
    {
      auto symbol = event.key.keysym.sym;

      keyState[symbol] = false;
      keyUpdate[symbol] = updateCounter;

      OnKeyRelease.Invoke(symbol);
    }

    // ===  CONTROLLERS

    // On controller connected
    else if (event.type == SDL_CONTROLLERDEVICEADDED)
    {
      MESSAGE << "Controller connected. Device index: " << event.cdevice.which << endl;

      // Try to open it
      try
      {
        OpenController(event.cdevice.which);
      }
      catch (const invalid_controller_error &error)
      {
        MESSAGE << "WARNING: " << string(error) << endl;
      }
    }

    // On controller disconnected
    else if (event.type == SDL_CONTROLLERDEVICEREMOVED)
    {
      MESSAGE << "Controller disconnected. Instance id: " << event.cdevice.which << endl;

      // Remove this entry
      controllers.erase(event.cdevice.which);
    }

    // On controller remapped
    else if (event.type == SDL_CONTROLLERDEVICEREMAPPED)
      MESSAGE << "Controller remapped. Instance id: " << event.cdevice.which << endl;

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
    {
      // If they are different, raise
      OnControllerLeftAnalog.Invoke(analog, instanceId);
    }
  }
  for (auto [instanceId, analog] : currentRightControllerAnalogs)
  {
    // Compare to the analog state last frame
    if (analog != lastRightControllerAnalogs[instanceId])
    {
      // If they are different, raise
      OnControllerRightAnalog.Invoke(analog, instanceId);
    }
  }

  // Store for next frame
  lastLeftControllerAnalogs = currentLeftControllerAnalogs;
  lastRightControllerAnalogs = currentRightControllerAnalogs;

  return eventPollingDelay;
}

bool PollEventsTimed(SDL_Event *event, float &timer)
{
  auto start = SDL_GetTicks();
  bool eventFound = SDL_PollEvent(event);
  auto end = SDL_GetTicks();

  // Gets duration as seconds
  float length = float(end - start) / 1000.0f;

  // Add duration to timer
  if (length > maximumPollDuration)
    timer += length - maximumPollDuration;

  return eventFound;
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

int InputManager::KeyStateLength(int key) const
{
  if (keyUpdate.count(key) == 0)
    return updateCounter;

  return updateCounter - keyUpdate.at(key);
}
