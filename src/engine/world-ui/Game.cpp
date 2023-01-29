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
#include "GameScene.h"
#include "MainScene.h"

using namespace std;
using namespace Helper;

class no_scene_error : runtime_error
{
public:
  no_scene_error(string message) : runtime_error(message) {}
};

// === EXTERNAL FIELDS

#ifdef DISPLAY_REAL_FPS
// How many frames have been rendered in the last second
static int framesThisSecond{0};

// How many physics frames have been processed in the last second
static int physicsFramesThisSecond{0};

// Counter (in ms) for counting how many frames are being rendered per second
static int secondCounter{0};
#endif

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

// Path to engine's default font
const string Game::defaultFontPath{"assets/engine/fonts/PixelOperator.ttf"};

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

  // === INITIALIZE SCENE

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

  // Apply discount
  Assert(deltaTime >= 0, "Delta time calculation failed: got a negative number");

  // Update frame start variable
  start = newStart;
}

#ifdef DISPLAY_REAL_FPS
void Game::CountElapsedFrames(int elapsedMilliseconds)
{
  secondCounter += elapsedMilliseconds;

  if (secondCounter >= 1000)
  {
    // Reset counter
    secondCounter = 0;

    framesInLastSecond = framesThisSecond;
    physicsFramesInLastSecond = physicsFramesThisSecond;
    framesThisSecond = 0;
    physicsFramesThisSecond = 0;
  }
}

void Game::DisplayRealFps()
{
  // Get text
  string text = to_string(framesInLastSecond) + "fps, " + to_string(physicsFramesInLastSecond) + "pfps";

  // Get font
  auto font = Resources::GetFont(defaultFontPath, 25);

  // Get texture

  // Will hold the generated surface
  static auto_unique_ptr<SDL_Surface> surface(nullptr, SDL_FreeSurface);

  // Use the appropriate method to load this
  surface.reset(TTF_RenderText_Solid(font.get(), text.c_str(), Color::Yellow()));

  // Ensure it's loaded
  Assert(surface != nullptr, "Failed to generate surface for frame counter font");

  // Convert to texture
  static auto_unique_ptr<SDL_Texture> texture(nullptr, SDL_DestroyTexture);

  texture.reset(SDL_CreateTextureFromSurface(GetRenderer(), surface.get()));

  // Get position
  static const int padding{10};
  static const Vector2 rawPosition{float(screenWidth) - padding, padding};
  // Offset coordinates to right align texture
  Vector2 position = rawPosition - Vector2{float(surface->w), 0};

  // Get the destination rect
  SDL_Rect destinationRect{int(position.x), int(position.y), surface->w, surface->h};

  // Get clip rectangle
  SDL_Rect clipRect{0, 0, surface->w, surface->h};

  // Put the texture in the renderer
  SDL_RenderCopyEx(
      GetRenderer(),
      texture.get(),
      &clipRect,
      &destinationRect,
      0,
      nullptr,
      SDL_FLIP_NONE);
}
#endif

// === PUBLIC METHODS =================================

Game &Game::GetInstance()
{
  // If it doesn't exist...
  if (gameInstance == nullptr)
  {
    // Create it
    gameInstance.reset(new Game("GuilhermeMendel-170143970", screenWidth, screenHeight));

    // Set a starting scene as next scene
    gameInstance->PushScene(gameInstance->GetInitialScene());
  }

  // Return the instance
  return *Game::gameInstance;
}

void Game::Start()
{
  // Push next scene in if necessary
  if (nextScene != nullptr)
    PushNextScene();

  started = true;

  // Start the initial scene
  GetScene()->Start();

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

  // Calculate how long each loop will take to execute
  int executionStart = SDL_GetTicks();

  // Loop while exit not requested
  while (GetScene()->QuitRequested() == false)
  {
#ifdef DISPLAY_REAL_FPS
    CountElapsedFrames(SDL_GetTicks() - executionStart);
#endif

    executionStart = SDL_GetTicks();

    try
    {
      if (nextPhysicsFrameIn <= 0)
      {
        PhysicsFrame();
        nextPhysicsFrameIn = physicsDelay;
      }

      if (nextFrameIn <= 0)
      {
        // Throws when trying to pop it's last scene (and no nextScene is set)
        Frame();
        nextFrameIn = frameDelay;
      }
    }
    catch (const no_scene_error &)
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

  // Make sure scene pile is empty
  while (loadedScenes.size() > 0)
    loadedScenes.pop();

  // Clear resources
  Resources::ClearAll();
}

void Game::Frame()
{
#ifdef PRINT_FRAME_DURATION
  float startMs = SDL_GetTicks();
#endif

#ifdef DISPLAY_REAL_FPS
  // Count this frame
  framesThisSecond++;
#endif
  // Objects to add to scene
  vector<shared_ptr<GameObject>> objectsToAdd;

  // Check if scene needs to be popped
  // Throws when it's the last scene (and no nextScene is set)
  if (GetScene()->PopRequested())
  {
    objectsToAdd = GetScene()->GetObjectsToCarryOn();
    PopScene();
  }

  // Load next scene if necessary
  if (nextScene != nullptr)
    PushNextScene();

  // Get reference to current scene
  GameScene &scene{*GetScene()};

  // Add new objects
  for (auto newObject : objectsToAdd)
    scene.RegisterObject(newObject);

  // Get input
  auto pollDelay = inputManager.Update();

  // Discount poll delay from delta times (convert seconds to ms)
  frameStart += pollDelay * 1000;
  physicsFrameStart += pollDelay * 1000;

  // Calculate frame's delta time
  CalculateDeltaTime(frameStart, deltaTime);

  // Update the scene's timer
  scene.timer.Update(deltaTime);

  // Update the scene
  scene.Update(deltaTime);

  // Render the scene
  scene.Render();

#ifdef DISPLAY_REAL_FPS
  // Render frame count
  DisplayRealFps();
#endif

  // WARNING: DO NOT USE scene FROM HERE UNTIL END OF LOOP

  // Render the window
  SDL_RenderPresent(GetRenderer());

#ifdef PRINT_FRAME_DURATION
  cout << "Frame took " << float(SDL_GetTicks()) - startMs << " ms" << endl;
#endif
}

void Game::PhysicsFrame()
{
#ifdef PRINT_FRAME_DURATION
  float startMs = SDL_GetTicks();
#endif

#ifdef DISPLAY_REAL_FPS
  // Count this frame
  physicsFramesThisSecond++;
#endif

  // Calculate physics frame's delta time
  CalculateDeltaTime(physicsFrameStart, physicsDeltaTime);

  // cout << "Physics delta time: " << physicsDeltaTime << endl;

  // Update the scene
  GetScene()->PhysicsUpdate(physicsDeltaTime);

#ifdef PRINT_FRAME_DURATION
  cout << "Physics took " << float(SDL_GetTicks()) - startMs << " ms" << endl;
#endif
}

shared_ptr<GameScene> Game::GetScene()
{
  Assert(loadedScenes.size() > 0, "No game scene loaded");

  return loadedScenes.top();
}

void Game::PushScene(shared_ptr<GameScene> &&scene)
{
  // Alert if next is overridden
  if (nextScene != nullptr)
    cout << "WARNING: call to " << __FUNCTION__ << " will override previous call in the same frame" << endl;

  // Store this scene for next frame
  nextScene = move(scene);
}

void Game::PushNextScene()
{
  Assert(nextScene != nullptr, "Failed to push next scene: it was nullptr");

  // Put current scene on hold
  if (loadedScenes.size() > 0)
    GetScene()->Pause();

  // Move this scene to the stack
  loadedScenes.emplace(move(nextScene));

  // Start it if necessary
  if (started)
    GetScene()->Start();
}

void Game::PopScene()
{
  // Remove the scene
  loadedScenes.pop();

  // If this is the last scene
  if (loadedScenes.size() == 0)
  {
    // Throws if there is no nextScene
    if (nextScene == nullptr)
      throw no_scene_error("Game was left without any loaded scenes");

    return;
  }

  // Resume next scene
  GetScene()->Resume();
}
