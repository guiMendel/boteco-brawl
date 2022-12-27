#ifndef __COMPONENT__
#define __COMPONENT__

#include "InputManager.h"
#include "Vector2.h"
#include "RenderLayer.h"
#include "ComponentParameter.h"
#include "SatCollision.h"
#include <string>
#include <memory>
#include <map>

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

  // Returns this component's shared pointer
  std::shared_ptr<Component> GetShared() const;

  void SetEnabled(bool enabled) { this->enabled = enabled; }
  bool IsEnabled() const;

  // The associated game state
  std::shared_ptr<GameState> GetState() const;

  // The associated game object
  GameObject &gameObject;

  // Whether the component is active
  bool enabled{true};

  // Component's unique name
  const int id;

protected:
  virtual void Awake() {}
  virtual void Start() {}
  virtual void RegisterToState() {}

  // Called once per frame
  virtual void Update(float) {}
  virtual void PhysicsUpdate(float) {}

  // Called once per frame to render to the screen
  virtual void Render() {}

  virtual void OnStateResume() {}
  virtual void OnStatePause() {}

  // Called on the frame it is destroyed, right before being destroyed
  virtual void OnBeforeDestroy() {}

  // Allows for reacting to collision
  virtual void OnCollision(Collision::CollisionData) {}
  virtual void OnCollisionEnter(Collision::CollisionData) {}
  virtual void OnCollisionExit(GameObject &) {}

  // Allows for reacting to trigger collision
  virtual void OnTriggerCollision(GameObject &) {}
  virtual void OnTriggerCollisionEnter(GameObject &) {}
  virtual void OnTriggerCollisionExit(GameObject &) {}

  // Reference to input manager
  InputManager &inputManager;

private:
  void RegisterLayer();
  void SafeStart();

  // Allows for registering to the state's variables
  void RegisterToStateWithLayer();

  // Whether Start has been called already
  bool started{false};
};

#include "GameObject.h"
// #include "GameState.h"
#include "Helper.h"

// We don't care it's recommended against doing this — we only want our lives to be easier
using namespace Helper;

#endif