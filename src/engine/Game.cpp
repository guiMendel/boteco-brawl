#include <iostream>
#include <string>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <cstdlib>
#include <ctime>
#include "Game.h"
#include "Helper.h"
#include "Resources.h"
#include "InputManager.h"
#include "TitleState.h"

using namespace std;
using namespace Helper;

// Defines the maximum frames per second
const int Game::frameRate{30};

// Defines the resolution width
const int Game::screenWidth{1024};

// Defines the resolution height
const int Game::screenHeight{600};

// === EXTERNAL METHODS =================================

// Initializes SDL
auto InitializeSDL(string title, int width, int height) -> pair<SDL_Window *, SDL_Renderer *>
{
  // === BASE SDL

  // Initialize SDL & all it's necessary subsystems
  auto encounteredError = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

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

void Game::CalculateDeltaTime()
{
  // Get this frame's start time
  float newFrameStart = SDL_GetTicks();

  // Calculate & convert delta time from ms to s
  deltaTime = (newFrameStart - frameStart) / 1000;

  // Update frame start variable
  frameStart = newFrameStart;
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
  // Find out how many ms to wait to achieve the configured framerate
  const int frameDelay = 1000 / Game::frameRate;

  // Get the input manager
  InputManager &inputManager = InputManager::GetInstance();

  // Push next state in if necessary
  if (nextState != nullptr)
    PushNextState();

  started = true;

  // Start the initial state
  GetState().Start();

  // Loop while exit not requested
  while (GetState().QuitRequested() == false)
  {
    // Check if state needs to be popped
    // Throws when it's the last state (and no nextState is set)
    try
    {
      if (GetState().PopRequested())
        PopState();
    }

    // If last state was popped, stop game
    catch (const runtime_error &)
    {
      break;
    }

    // Load next state if necessary
    if (nextState != nullptr)
      PushNextState();

    // Get reference to current state
    GameState &state{GetState()};

    // Calculate frame's delta time
    CalculateDeltaTime();

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

    // Delay the frame to obey the framerate
    SDL_Delay(frameDelay);
  }

  // Make sure state pile is empty
  while (loadedStates.size() > 0)
    loadedStates.pop();

  // Clear resources
  Resources::ClearAll();
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
    Assert(nextState != nullptr, "Game was left without any loaded states");

    return;
  }

  // Resume next state
  GetState().Resume();
}

unique_ptr<GameState> Game::GetInitialState() const
{
  return make_unique<TitleState>();
}
