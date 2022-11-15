#ifndef __INPUT_MANAGER__
#define __INPUT_MANAGER__

#include "Vector2.h"
#include <iostream>
#include <unordered_map>
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
  // Singleton access using Meyer's implementation
  static InputManager &GetInstance()
  {
    static InputManager instance;

    return instance;
  }

  void Update();

  bool KeyPress(int key) { return keyState[key] == true && keyUpdate[key] == updateCounter; }
  bool KeyRelease(int key) { return keyState[key] == false && keyUpdate[key] == updateCounter; }

  // When the key does not exist, a new element is constructed with the default constructor and inserted (in this case, it gets the false value)
  bool IsKeyDown(int key) { return keyState[key]; }

  bool MousePress(int button) { return mouseState[button] == true && mouseUpdate[button] == updateCounter; }
  bool MouseRelease(int button)
  {
    return mouseState[button] == false && mouseUpdate[button] == updateCounter;
  }

  bool IsMouseDown(int button) { return mouseState[button]; }

  int GetMouseX() const { return mouseX; }
  int GetMouseY() const { return mouseY; }
  Vector2 GetMouseWorldCoordinates() const;

  bool QuitRequested() const { return quitRequested; }

private:
  // Default constructor
  // No need for constructor since all values were initialized in class definition
  InputManager() {}

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
};

#endif