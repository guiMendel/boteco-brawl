#ifndef __GAME__
#define __GAME__

#include <SDL.h>
#include <memory>
#include <stack>
#include "Helper.h"

class GameState;

// Class with the main game logic
class Game
{
public:
  // === CONFIGURATION

  // Defines the maximum frames per second
  static const int frameRate;

  // Defines the resolution width
  static const int screenWidth;

  // Defines the resolution height
  static const int screenHeight;

  // === FUNCTIONS

  // Gets the game instance if it exists or creates one if it doesn't
  static Game &GetInstance();

  // Gets the current game state
  GameState &GetState() const;

  // Gets the renderer
  SDL_Renderer *GetRenderer() const { return renderer.get(); }

  // Starts the game
  void Start();

  float GetDeltaTime() const { return deltaTime; }

  // Requests the push of a new state to the queue
  void PushState(std::unique_ptr<GameState> &&state);

  // Creates and returns the initial state
  std::unique_ptr<GameState> GetInitialState() const;

  // Explicit destructor
  ~Game();

private:
  // Singleton constructor
  Game(std::string title, int width, int height);

  // Calculates the delta time
  void CalculateDeltaTime();

  // Removes current state from stack
  // Throws if stack is left empty
  void PopState();

  // Actually pushes the next state to stack
  void PushNextState();

  // Game instance
  static std::unique_ptr<Game> gameInstance;

  // Start time of current frame, in milliseconds
  int frameStart{(int)SDL_GetTicks()};

  // Time elapsed since last frame
  float deltaTime;

  // Whether game has started
  bool started{false};

  // State to push next frame
  std::unique_ptr<GameState> nextState;

  // Stack of loaded states
  std::stack<std::unique_ptr<GameState>> loadedStates;

  // The window we'll be rendering to (with destructor function)
  Helper::auto_unique_ptr<SDL_Window> window;

  // Renderer for the window (with destructor function)
  Helper::auto_unique_ptr<SDL_Renderer> renderer;
};

#include "GameState.h"

#endif