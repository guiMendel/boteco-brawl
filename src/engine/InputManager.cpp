#include "InputManager.h"
#include "Camera.h"
#include <SDL.h>

using namespace std;

// No need for constructor since all values were initialized in class definition

void InputManager::Update()
{
  SDL_Event event;

  // Get mouse coords
  SDL_GetMouseState(&mouseX, &mouseY);

  // Reset quit request
  quitRequested = false;

  // Increment counter
  updateCounter++;

  // If there are any input events in the SDL stack pile, this function returns 1 and sets the argument to next event
  while (SDL_PollEvent(&event))
  {
    // Quit on quit event
    if (event.type == SDL_QUIT)
      quitRequested = true;

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

    // On keyboard event
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

    // On keyboard event
    else if (event.type == SDL_KEYUP)
    {
      auto symbol = event.key.keysym.sym;

      keyState[symbol] = false;
      keyUpdate[symbol] = updateCounter;
    }
  }
}

Vector2 InputManager::GetMouseWorldCoordinates() const
{
  return Camera::GetInstance().ScreenToWorld(Vector2((float)mouseX, (float)mouseY));
}
