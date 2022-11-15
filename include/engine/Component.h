#ifndef __COMPONENT__
#define __COMPONENT__

#include "InputManager.h"
#include "Vector2.h"
#include "RenderLayer.h"
#include <string>
#include <memory>

#define LOCK(weak, shared)   \
  auto shared = weak.lock(); \
  Assert(shared != nullptr,  \
         "Unexpectedly failed to lock shared pointer at " __FILE__ ":" + std::to_string(__LINE__) + " ");

#define LOCK_MESSAGE(weak, shared, message) \
  auto shared = weak.lock();                \
  Assert(shared != nullptr, message);

class GameObject;
class GameState;

class Component
{
  friend GameObject;
  friend GameState;

public:
  Component(GameObject &associatedObject);
  virtual ~Component() {}

  // In which render layer this component is
  // If None, then it's Render method will never be called
  virtual RenderLayer GetRenderLayer() { return RenderLayer::None; }

  // The order in which to render this component in it's layer (higher numbers are shown on top)
  virtual int GetRenderOrder() { return 0; }

  void StartAndRegisterLayer();

  // Returns this component's shared pointer
  std::shared_ptr<Component> GetShared() const;

  void SetEnabled(bool enabled) { this->enabled = enabled; }
  bool IsEnabled() const;

  // The associated game object
  GameObject &gameObject;

  // The associated game state
  GameState &gameState;

  // Whether the component is active
  bool enabled{true};

protected:
  virtual void Start() {}

  // Called once per frame
  virtual void Update([[maybe_unused]] float deltaTime) {}

  // Called once per frame to render to the screen
  virtual void Render() {}

  virtual void OnStateResume() {}
  virtual void OnStatePause() {}

  // Called on the frame it is destroyed, right before being destroyed
  virtual void OnBeforeDestroy() {}

  // Reference to input manager
  InputManager &inputManager;

private:
  // Allows for reacting to collision
  virtual void OnCollision([[maybe_unused]] GameObject &other) {}

  // Whether StartAndRegisterLayer has been called already
  bool started{false};
};

#include "GameObject.h"
// #include "GameState.h"
#include "Helper.h"

// We don't care it's recommended against doing this — we only want our lives to be easier
using namespace Helper;

#endif