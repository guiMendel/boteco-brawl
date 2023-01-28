#ifndef __COMPONENT__
#define __COMPONENT__

#include "InputManager.h"
#include "Vector2.h"
#include "RenderLayer.h"
#include "ComponentParameter.h"
#include "Collision.h"
#include "TriggerCollisionData.h"
#include <string>
#include <memory>
#include <map>

#define IF_LOCK(weak, shared) if (auto shared = weak.lock(); shared)

#define IF_NOT_LOCK(weak, shared) \
  auto shared = weak.lock();      \
  if (shared == nullptr)

#define LOCK(weak, shared)   \
  auto shared = weak.lock(); \
  Helper::Assert(shared != nullptr,  \
         "Unexpectedly failed to lock shared pointer at " __FILE__ ":" + std::to_string(__LINE__) + " ");

#define LOCK_MESSAGE(weak, shared, message) \
  auto shared = weak.lock();                \
  Helper::Assert(shared != nullptr, message);

class WorldObject;
class GameScene;

class Component
{
  friend WorldObject;
  friend GameScene;

public:
  Component(WorldObject &associatedObject);
  virtual ~Component();

  // In which render layer this component is
  // If None, then it's Render method will never be called
  virtual RenderLayer GetRenderLayer() { return RenderLayer::None; }

  // The order in which to render this component in it's layer (higher numbers are shown on top)
  virtual int GetRenderOrder() { return 0; }

  // Returns this component's shared pointer
  std::shared_ptr<Component> GetShared() const;

  void SetEnabled(bool enabled) { this->enabled = enabled; }
  bool IsEnabled() const;

  // The associated game scene
  std::shared_ptr<GameScene> GetScene() const;

  bool HasCalledStart() const;

  bool operator==(const Component &other) const;

  explicit operator std::string() const;

  // The associated world object
  WorldObject &worldObject;

  // Whether the component is active
  bool enabled{true};

  // Component's unique identifier
  const int id;

protected:
  virtual void Awake() {}
  virtual void Start() {}
  virtual void RegisterToScene() {}

  // Called once per frame
  virtual void Update(float) {}
  virtual void PhysicsUpdate(float) {}

  // Called once per frame to render to the screen
  virtual void Render() {}

  virtual void OnSceneResume() {}
  virtual void OnScenePause() {}

  // Called on the frame it is destroyed, right before being destroyed
  virtual void OnBeforeDestroy() {}

  // Allows for reacting to collision
  virtual void OnCollision(Collision::Data) {}
  virtual void OnCollisionEnter(Collision::Data) {}
  virtual void OnCollisionExit(Collision::Data) {}

  // Allows for reacting to trigger collision
  virtual void OnTriggerCollision(TriggerCollisionData) {}
  virtual void OnTriggerCollisionEnter(TriggerCollisionData) {}
  virtual void OnTriggerCollisionExit(TriggerCollisionData) {}

  // Reference to input manager
  InputManager &inputManager;

private:
  void RegisterLayer();
  void SafeStart();

  // Allows for registering to the state's variables
  void RegisterToSceneWithLayer();

  // Whether Start has been called already
  bool started{false};
};

std::ostream &operator<<(std::ostream &stream, const Component &vector);

#include "WorldObject.h"
// #include "GameScene.h"
#include "Helper.h"

// We don't care it's recommended against doing this — we only want our lives to be easier
using namespace Helper;

#endif