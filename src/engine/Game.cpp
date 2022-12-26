#include <iostream>
#include <string>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <SDL_gamecontroller.h>
#include <cstdlib>
#include <ctime>
#include "Game.h"
#include "Helper.h"
#include "Resources.h"
#include "GameState.h"
#include "MainState.h"

using namespace std;
using namespace Helper;

class no_state_error : runtime_error
{
public:
  no_state_error(string message) : runtime_error(message) {}
};

// === EXTERNAL METHODS =================================

// Initializes SDL
auto InitializeSDL(string title, int width, int height) -> pair<SDL_Window *, SDL_Renderer *>
{
  // === BASE SDL

  // Initialize SDL & all it's necessary subsystems
  auto encounteredError = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER);

  // Catch any errors
  Assert(!encounteredError, "Failed to initialize SDL");

  // === SDL IMAGE

  // Initialize the image module
  int requestedFlags = IMG_INIT_JPG | IMG_INIT_PNG;
  int returnedFlags = IMG_Init(requestedFlags);

  // Check if everything went alright
  Assert((returnedFlags & requestedFlags) == requestedFlags, "Failed to initialize SDL-image", IMG_GetError());

  // === SDL MIXER

  // Initialize the mixer
  Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3);

  // Initialize open audio
  encounteredError = Mix_OpenAudio(
      MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);

  // Catch any errors
  Assert(!encounteredError, "Failed to initialize SDL-mixer", Mix_GetError());

  // Allocate more sound channels
  Mix_AllocateChannels(32);

  // === SDL FONTS

  // Ensure initializing works
  Assert(TTF_Init() == 0, "Failed to initialize SDL-ttf", TTF_GetError());

  // === GAME WINDOW

  auto gameWindow = SDL_CreateWindow(
      title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);

  // Catch any errors
  Assert(gameWindow != nullptr, "Failed to create SDL window");

  // Create renderer
  auto renderer = SDL_CreateRenderer(gameWindow, -1, SDL_RENDERER_ACCELERATED);

  // Catch any errors
  Assert(renderer != nullptr, "Failed to create SDL renderer");

  // === CONTROLLERS

  // Enable controller events
  SDL_GameControllerEventState(SDL_ENABLE);

  return make_pair(gameWindow, renderer);
}

void ExitSDL(SDL_Window *window, SDL_Renderer *renderer)
{
  SDL_DestroyRenderer(renderer);

  SDL_DestroyWindow(window);

  TTF_Quit();

  Mix_CloseAudio();

  Mix_Quit();

  IMG_Quit();

  SDL_Quit();
}

// === INITIALIZE STATIC FIELDS =================================

// Game instance
unique_ptr<Game> Game::gameInstance = nullptr;

// === PRIVATE METHODS =======================================

Game::Game(string title, int width, int height)
    : window(nullptr, SDL_DestroyWindow), renderer(nullptr, SDL_DestroyRenderer)
{
  // === SINGLETON CHECK

  // Check for invalid existing instance
  if (Game::gameInstance)
    throw runtime_error("Tried to instantiate another Game instance");

  // === INIT SDL

  // Retrieve the window & the renderer from the initializer
  auto pointers = InitializeSDL(title, width, height);

  // === INITIALIZE STATE

  window.reset(pointers.first);
  renderer.reset(pointers.second);

  // === INIT RANDOMNESS

  srand(time(NULL));
}

Game::~Game()
{
  // Quit SDL
  // Release the pointers, as we will destroy them in the method
  ExitSDL(window.release(), renderer.release());
}

void Game::CalculateDeltaTime(int &start, float &deltaTime)
{
  // Get this frame's start time
  int newStart = SDL_GetTicks();

  // Calculate & convert delta time from ms to s
  deltaTime = (float)(newStart - start) / 1000.0f;

  // Update frame start variable
  start = newStart;
}

// === PUBLIC METHODS =================================

Game &Game::GetInstance()
{
  // If it doesn't exist...
  if (gameInstance == nullptr)
  {
    // Create it
    gameInstance.reset(new Game("GuilhermeMendel-170143970", screenWidth, screenHeight));

    // Set a starting state as next state
    gameInstance->PushState(gameInstance->GetInitialState());
  }

  // Return the instance
  return *Game::gameInstance;
}

void Game::Start()
{
  // Push next state in if necessary
  if (nextState != nullptr)
    PushNextState();

  started = true;

  // Start the initial state
  GetState().Start();

  GameLoop();
}

void Game::GameLoop()
{
  // Amount of milliseconds between each frame
  const int frameDelay = 1000 / Game::frameRate;

  // Amount of milliseconds between each physics frame
  const int physicsDelay = 1000 / Game::physicsFrameRate;

  // Milliseconds until next frame
  int nextFrameIn{0};

  // Milliseconds until next physics frame
  int nextPhysicsFrameIn{0};

  // Loop while exit not requested
  while (GetState().QuitRequested() == false)
  {
    // Calculate how long this loop will take to execute
    int executionStart = SDL_GetTicks();

    try
    {
      if (nextPhysicsFrameIn <= 0)
      {
        PhysicsFrame();
        nextPhysicsFrameIn = physicsDelay;
      }

      if (nextFrameIn <= 0)
      {
        // Throws when trying to pop it's last state (and no nextState is set)
        Frame();
        nextFrameIn = frameDelay;
      }
    }
    catch (const no_state_error &)
    {
      break;
    }

    // Check how long this loop was
    int loopDuration = (int)SDL_GetTicks() - executionStart;

    // Discount from each counter
    nextFrameIn -= loopDuration;
    nextPhysicsFrameIn -= loopDuration;

    // Check how long to wait until next loop
    int sleepTime = min(nextFrameIn, nextPhysicsFrameIn);

    if (sleepTime > 0)
    {
      // Discount sleep time
      nextFrameIn -= sleepTime;
      nextPhysicsFrameIn -= sleepTime;

      SDL_Delay(sleepTime);
    }
  }

  // Make sure state pile is empty
  while (loadedStates.size() > 0)
    loadedStates.pop();

  // Clear resources
  Resources::ClearAll();
}

void Game::Frame()
{
  // Check if state needs to be popped
  // Throws when it's the last state (and no nextState is set)
  if (GetState().PopRequested())
    PopState();

  // Load next state if necessary
  if (nextState != nullptr)
    PushNextState();

  // Get reference to current state
  GameState &state{GetState()};

  // Calculate frame's delta time
  CalculateDeltaTime(frameStart, deltaTime);

  // Get input
  inputManager.Update();

  // Update the state's timer
  state.timer.Update(deltaTime);

  // Update the state
  state.Update(deltaTime);

  // Render the state
  state.Render();

  // WARNING: DO NOT USE state FROM HERE UNTIL END OF LOOP

  // Render the window
  SDL_RenderPresent(GetRenderer());
}

void Game::PhysicsFrame()
{
  // Get reference to current state
  GameState &state{GetState()};

  // Calculate physics frame's delta time
  CalculateDeltaTime(physicsFrameStart, physicsDeltaTime);

  // Update the state
  state.PhysicsUpdate(physicsDeltaTime);
}

GameState &Game::GetState() const
{
  Assert(loadedStates.size() > 0, "No game state loaded");

  return *loadedStates.top();
}

void Game::PushState(std::unique_ptr<GameState> &&state)
{
  // Alert if next is overridden
  if (nextState != nullptr)
    cout << "WARNING: call to " << __FUNCTION__ << " will override previous call in the same frame" << endl;

  // Store this state for next frame
  nextState = move(state);
}

void Game::PushNextState()
{
  Assert(nextState != nullptr, "Failed to push next state: it was nullptr");

  // Put current state on hold
  if (loadedStates.size() > 0)
    GetState().Pause();

  // Move this state to the stack
  loadedStates.emplace(move(nextState));

  // Start it if necessary
  if (started)
    GetState().Start();
}

void Game::PopState()
{
  // Remove the state
  loadedStates.pop();

  // If this is the last state
  if (loadedStates.size() == 0)
  {
    // Throws if there is no nextState
    if (nextState == nullptr)
      throw no_state_error("Game was left without any loaded states");

    return;
  }

  // Resume next state
  GetState().Resume();
}
