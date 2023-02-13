#ifndef __COMPONENT__
#define __COMPONENT__

#include "InputManager.h"
#include "Vector2.h"
#include "RenderLayer.h"
#include "ComponentParameter.h"
#include "Color.h"
#include "Renderable.h"
#include <string>
#include <memory>

class WorldObject;
class ComponentOwner;
class GameObject;
class GameScene;
class UIContainer;
class UIContent;

// We don't care it's recommended against doing this — we only want our lives to be easier
using namespace Helper;

class Component : public Renderable
{
  friend ComponentOwner;
  friend GameObject;
  friend WorldObject;
  friend GameScene;
  friend UIContainer;
  friend UIContent;

public:
  Component(GameObject &associatedObject);

  virtual ~Component();

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

  // Called on the frame it is destroyed, right before being destroyed
  virtual void OnBeforeDestroy() {}

  // CAlled when the owner calls CascadeDown
  virtual void CascadeDown(std::function<void(GameObject &)>, bool) {}

  // Reference to input manager
  InputManager &inputManager;

private:
  // Registers this component's render layer if it is not None
  void RegisterLayer() override;

  bool ShouldRender() override;

  // Ensures Start never gets called more than once
  void SafeStart();

  // Allows for registering to the state's variables
  void RegisterToSceneWithLayer();

  // Whether Start has been called already
  bool started{false};
};

std::ostream &operator<<(std::ostream &stream, const Component &vector);

#endif