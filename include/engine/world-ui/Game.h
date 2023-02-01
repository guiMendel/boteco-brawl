#ifndef __GAME__
#define __GAME__

#include <SDL.h>
#include <memory>
#include <stack>
#include "Helper.h"
#include "InputManager.h"
#include "BuildConfigurations.h"

class GameScene;

// Class with the main game logic
class Game
{
public:
  // === CONFIGURATION

  // Defines the maximum frames per second
  static const int frameRate;

  // Defines the maximum physics frames per second
  static const int physicsFrameRate;

  // Defines the resolution width, in pixels
  static const int screenWidth;

  // Defines the resolution height, in pixels
  static const int screenHeight;

  // Default pixels per unit
  static const int defaultVirtualPixelsPerUnit;

  // === FUNCTIONS

  // Gets the game instance if it exists or creates one if it doesn't
  static Game &GetInstance();

  // Gets the current game scene
  std::shared_ptr<GameScene> GetScene();

  // Gets the renderer
  SDL_Renderer *GetRenderer() const { return renderer.get(); }

  // Starts the game
  void Start();

  float GetDeltaTime() const { return deltaTime; }
  float GetPhysicsDeltaTime() const { return physicsDeltaTime; }

  // Requests the push of a new scene to the queue
  void PushScene(std::shared_ptr<GameScene> &&scene);

  // Creates and returns the initial scene
  std::shared_ptr<GameScene> GetInitialScene() const;

  InputManager &GetInputManager() { return inputManager; }

  // Supplies a valid unique id for a world object or a component
  int SupplyId() { return nextId++; }

  // Explicit destructor
  virtual ~Game();

  // Path to engine's default font
  static const std::string defaultFontPath;

private:
  // Singleton constructor
  Game(std::string title, int width, int height);

  // Calculates the delta time
  void CalculateDeltaTime(int &start, float &deltaTime);

  // Calls destroy scene and resumes next scene
  // Throws if stack is left empty
  void PopScene();

  // Removes current scene from stack and destroys it
  void DestroyScene();

  // Actually pushes the next scene to stack
  void PushNextScene();

  void GameLoop();

  // Behavior of a frame
  void Frame();

  // Behavior of a physics frame
  void PhysicsFrame();

#ifdef DISPLAY_REAL_FPS
  // Counts elapsed frames this second
  void CountElapsedFrames(int elapsedMilliseconds);

  // Displays actual fps on top left corner of screen
  void DisplayRealFps();
#endif

  // Game instance
  static std::unique_ptr<Game> gameInstance;

  // ID counter for world objects and components
  int nextId{1};

  // Start time of current frame, in milliseconds
  int frameStart{(int)SDL_GetTicks()};

  // Time elapsed since last frame
  float deltaTime;

  // Start time of current physics frame, in milliseconds
  int physicsFrameStart{(int)SDL_GetTicks()};

  // Time elapsed since last physics frame
  float physicsDeltaTime;

  // Whether game has started
  bool started{false};

#ifdef DISPLAY_REAL_FPS
  // How many frames have actually rendered last second
  int framesInLastSecond{0};

  // How many physics frames have actually rendered last second
  int physicsFramesInLastSecond{0};
#endif

  // Input manager instance
  InputManager inputManager;

  // Scene to push next frame
  std::shared_ptr<GameScene> nextScene;

  // Stack of loaded scenes
  std::stack<std::shared_ptr<GameScene>> loadedScenes;

  // The window we'll be rendering to (with destructor function)
  Helper::auto_unique_ptr<SDL_Window> window;

  // Renderer for the window (with destructor function)
  Helper::auto_unique_ptr<SDL_Renderer> renderer;
};

#endif