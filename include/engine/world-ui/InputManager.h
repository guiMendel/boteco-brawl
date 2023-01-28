#ifndef __INPUT_MANAGER__
#define __INPUT_MANAGER__

#include "Vector2.h"
#include "Event.h"
#include "Helper.h"
#include "ControllerDevice.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <SDL.h>

#define LEFT_ARROW_KEY SDLK_LEFT
#define RIGHT_ARROW_KEY SDLK_RIGHT
#define UP_ARROW_KEY SDLK_UP
#define DOWN_ARROW_KEY SDLK_DOWN
#define ESCAPE_KEY SDLK_ESCAPE
#define SPACE_KEY SDLK_SPACE
#define LEFT_MOUSE_BUTTON SDL_BUTTON_LEFT
#define RIGHT_MOUSE_BUTTON SDL_BUTTON_RIGHT

class InputManager
{
public:
  // Controller analog movement, with Controller instance id
  // Vector axis range from -1 to 1
  EventII<Vector2, int> OnControllerLeftAnalog, OnControllerRightAnalog;

  // Controller button event, with button & Controller instance id
  EventII<SDL_GameControllerButton, int> OnControllerButtonPress, OnControllerButtonRelease;

  // Poll SDL events
  // Returns how many seconds the SDL_PollEvent function took to execute, in total
  // Reason: SDL_PollEvent may suspend the execution if user drags or resizes screen
  float Update();

  bool KeyPress(int key) { return keyState[key] == true && keyUpdate[key] == updateCounter; }
  bool KeyRelease(int key) { return keyState[key] == false && keyUpdate[key] == updateCounter; }

  // When the key does not exist, a new element is constructed with the default constructor and inserted (in this case, it gets the false value)
  bool IsKeyDown(int key) { return keyState[key]; }

  // Get how many frames this key has been in it's state for
  int KeyStateLength(int key) const;

  bool MousePress(int button) { return mouseState[button] == true && mouseUpdate[button] == updateCounter; }
  bool MouseRelease(int button)
  {
    return mouseState[button] == false && mouseUpdate[button] == updateCounter;
  }

  bool IsMouseDown(int button) { return mouseState[button]; }

  int GetMouseX() const { return mouseX; }
  int GetMouseY() const { return mouseY; }
  Vector2 GetMouseWorldCoordinates() const;

  std::shared_ptr<ControllerDevice> GetController(int id);

  bool QuitRequested() const { return quitRequested; }

private:
  // Flattens joystick axis value in range -1 to 1, and sets it to 0 if below deadzone
  float TreatAxisValue(int value);

  // Opens a new controller with the given index
  void OpenController(int index);

  // Mouse state flags
  bool mouseState[6]{};

  int mouseUpdate[6]{};

  // Keyboard state flags
  std::unordered_map<int, bool> keyState;

  std::unordered_map<int, int> keyUpdate;

  // Whether user has requested to quit
  bool quitRequested{false};

  int updateCounter;

  // Mouse X coordinates
  int mouseX;

  // Mouse Y coordinates
  int mouseY;

  // Currently open controllers
  std::unordered_map<int, std::shared_ptr<ControllerDevice>> controllers;

  // Tolerance of joystick axis jitter, in range 0 to 1
  static const float joystickDeadZone;
};

#endif