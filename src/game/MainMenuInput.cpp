#include "MainMenuInput.h"
#include "InputManager.h"

using namespace std;

MainMenuInput::MainMenuInput(GameObject &associatedObject)
    : WorldComponent(associatedObject) {}

void MainMenuInput::Start()
{
  // Raises stat if enter was pressed
  auto onKeyPress = [this](int key)
  {
    if (key == SDLK_RETURN)
      HitStart();
  };

  inputManager.OnKeyPress.AddListener("ui-start", onKeyPress);

  // Raises stat if start was pressed
  auto onButtonPress = [this](SDL_GameControllerButton button, int)
  {
    if (button == SDL_CONTROLLER_BUTTON_START)
      HitStart();
  };

  inputManager.OnControllerButtonPress.AddListener("ui-start", onButtonPress);
}

void MainMenuInput::HitStart()
{
  cout << "GOOOOO" << endl;
}
