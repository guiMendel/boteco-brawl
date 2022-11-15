#include "TitleState.h"
#include "Recipes.h"
#include "Game.h"
#include "MainState.h"

using namespace std;

const float TitleState::instructionFlashTime{1.0f};

void TitleState::InitializeObjects()
{
  // Add a background
  CreateObject("Background", Recipes::Background("./assets/image/title.jpg"));

  // Add a text instruction
  auto instruction = CreateObject("Instruction", Recipes::Text("Press Space to start", 50, Color(192, 180, 16)), Vector2(0, 200));
  instruction->timer.Reset("flash", -instructionFlashTime);
  instructionWeak = instruction;
}

void TitleState::Update(float deltaTime)
{
  // Call base
  GameState::Update(deltaTime);

  // Check if needs to change instruction visibility
  LOCK(instructionWeak, instruction);
  if (instruction->timer.Get("flash") >= 0)
  {
    instruction->SetEnabled(!instruction->IsEnabled());
    instruction->timer.Reset("flash", -instructionFlashTime);
  }

  // Quit on esc key
  if (inputManager.KeyRelease(ESCAPE_KEY))
  {
    quitRequested = true;
  }

  // Start main state on space key
  else if (inputManager.KeyPress(SPACE_KEY))
  {
    Game::GetInstance().PushState(make_unique<MainState>());
  }
}
