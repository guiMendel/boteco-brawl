#include "MainMenuInput.h"
#include "InputManager.h"

using namespace std;

MainMenuInput::MainMenuInput(GameObject &associatedObject, shared_ptr<SplashAnimation> animationHandler)
    : WorldComponent(associatedObject), weakAnimationHandler(animationHandler) {}

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
  // Get animation handler
  LOCK(weakAnimationHandler, animationHandler);

  // Pan to selection screen
  animationHandler->PanContent(1);

  // Reset any active animations
  animationHandler->ResetInitialAnimation();
}
