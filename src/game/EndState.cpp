#include "EndState.h"
#include "Recipes.h"
#include "Game.h"
#include "MainState.h"
#include "GameData.h"

using namespace std;

void EndState::InitializeObjects()
{
  // Whether player won
  bool playerWon = GameData::GetInstance().playerWon;

  string background = playerWon ? "./assets/image/win.jpg" : "./assets/image/lose.jpg";
  string text = playerWon ? "Well done!" : "Try again!";
  string musicPath = playerWon ? "./assets/music/endStateWin.ogg" : "./assets/music/endStateLose.ogg";

  // Add a background
  CreateObject("Background", Recipes::Background(background));

  // Add a message
  CreateObject("Instruction", Recipes::Text(text, 50, Color(192, 180, 16)), Vector2(0, 250));

  // Play music
  music.Play(musicPath);
}

void EndState::Update(float deltaTime)
{
  // Call base
  GameState::Update(deltaTime);

  // Quit on esc key
  if (inputManager.KeyRelease(ESCAPE_KEY))
  {
    quitRequested = true;
  }

  // Pop on esc key and load main
  else if (inputManager.KeyPress(SPACE_KEY))
  {
    Game::GetInstance().PushState(make_unique<MainState>());
    popRequested = true;
  }
}
