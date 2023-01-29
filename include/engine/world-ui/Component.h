#ifndef __COMPONENT__
#define __COMPONENT__

#include "InputManager.h"
#include "Vector2.h"
// TODO: remove render logic from component and bring it to it's own interface called Renderable
// TODO: remove event hooks and bring them to their own interface
#include "RenderLayer.h"
#include "ComponentParameter.h"
#include "Color.h"
#include <string>
#include <memory>

class WorldObject;
class GameObject;
class GameScene;

// We don't care it's recommended against doing this — we only want our lives to be easier
using namespace Helper;

class Component
{
  friend GameObject;
  friend WorldObject;
  friend GameScene;

public:
  Component(GameObject &associatedObject);

  virtual ~Component();

  // In which render layer this component is
  // If None, then it's Render method will never be called
  virtual RenderLayer GetRenderLayer() { return RenderLayer::None; }

  // The order in which to render this component in it's layer (higher numbers are shown on top)
  virtual int GetRenderOrder() { return 0; }

  void SetEnabled(bool value);
  bool IsEnabled() const;

  // The associated game scene
  std::shared_ptr<GameScene> GetScene() const;

  bool HasCalledStart() const;

  bool operator==(const Component &other) const;
  bool operator!=(const Component &other) const;

  explicit operator std::string() const;

  // The associated game object
  GameObject &gameObject;

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

  // Reference to input manager
  InputManager &inputManager;

private:
  // Registers this component's render layer if it is not None
  void RegisterLayer();

  // Ensures Start never gets called more than once
  void SafeStart();

  // Allows for registering to the state's variables
  void RegisterToSceneWithLayer();

  // Whether Start has been called already
  bool started{false};
};

std::ostream &operator<<(std::ostream &stream, const Component &vector);

#endif