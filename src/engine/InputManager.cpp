#include "InputManager.h"
#include "Camera.h"
#include <SDL.h>

#define CONTROLLER_AXIS_MAX 32767.0f

using namespace std;

const int InputManager::joystickDeadZone{8000};

// No need for constructor since all values were initialized in class definition

void InputManager::Update()
{
  SDL_Event event;

  // Get mouse coords
  SDL_GetMouseState(&mouseX, &mouseY);

  // Handle changes to connected controllers
  ConnectControllers();

  // Reset quit request
  quitRequested = false;

  // Increment counter
  updateCounter++;

  // Game controller mapping for analogues in last frame
  static unordered_map<int, Vector2> lastLeftControllerAnalogues, lastRightControllerAnalogues;

  // Game controller mapping for analogues in this frame
  static unordered_map<int, Vector2> currentLeftControllerAnalogues, currentRightControllerAnalogues;

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
      cout << "Controller connected. Device index: " << event.cdevice.which << endl;

    // On controller disconnected
    else if (event.type == SDL_CONTROLLERDEVICEREMOVED)
      cout << "Controller disconnected. Instance id: " << event.cdevice.which << endl;

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
      // Register this axis variation for this frame
      if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
        currentLeftControllerAnalogues[event.caxis.which].x = float(event.caxis.value) / CONTROLLER_AXIS_MAX;

      if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
        currentLeftControllerAnalogues[event.caxis.which].y = float(event.caxis.value) / CONTROLLER_AXIS_MAX;

      if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
        currentRightControllerAnalogues[event.caxis.which].x = float(event.caxis.value) / CONTROLLER_AXIS_MAX;

      if (event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
        currentRightControllerAnalogues[event.caxis.which].y = float(event.caxis.value) / CONTROLLER_AXIS_MAX;
    }
  }

  // Detect variation of controller analogue
  for (auto [instanceId, analogue] : currentLeftControllerAnalogues)
  {
    // Compare to the analogue state last frame
    if (analogue != lastLeftControllerAnalogues[instanceId])
      // If they are different, raise
      OnControllerLeftAnalogue.Invoke(analogue, instanceId);
  }
  for (auto [instanceId, analogue] : currentRightControllerAnalogues)
  {
    // Compare to the analogue state last frame
    if (analogue != lastRightControllerAnalogues[instanceId])
      // If they are different, raise
      OnControllerRightAnalogue.Invoke(analogue, instanceId);
  }

  // Store for next frame
  lastLeftControllerAnalogues = currentLeftControllerAnalogues;
  lastRightControllerAnalogues = currentRightControllerAnalogues;

  // Empty these for next frame
  currentLeftControllerAnalogues.clear();
  currentRightControllerAnalogues.clear();
}

void InputManager::ConnectControllers()
{
  // Get how many joysticks are connected to the system
  int connectedJoystickCount = SDL_NumJoysticks();

  // Catch errors
  Assert(connectedJoystickCount >= 0, "Failed to retrieve current number of connected joysticks");

  // If there are new joysticks, open them
  while (connectedJoystickCount > controllers.size())
  {
    // For now, let's enforce joysticks which are also game controllers
    Assert(SDL_IsGameController(controllers.size()), "Connected game controller was not recognized");

    // Create it's unique ptr with a destructor
    controllers.emplace_back(SDL_GameControllerOpen(controllers.size()), SDL_GameControllerClose);

    // Check for it's health
    Assert(controllers[controllers.size() - 1] != nullptr, "Failed to open newly connected controller");
  }

  // If joysticks have been removed, close them
  while (connectedJoystickCount < controllers.size())
    controllers.pop_back();
}

Vector2 InputManager::GetMouseWorldCoordinates() const
{
  return Camera::GetMain()->ScreenToWorld(Vector2((float)mouseX, (float)mouseY));
}
