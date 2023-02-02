#ifndef __CANVAS__
#define __CANVAS__

#include "WorldComponent.h"
#include "Camera.h"

class UIContainer;

// Creates a space onto which UI can be inserted
class Canvas : public WorldComponent
{
  friend class UIContainer;

public:
  // Different types of canvas
  enum class Space
  {
    // Takes up the whole space of the screen, and is completely disconnected from the game world
    Global,

    // Has a position in the game world, but it's size is independent of it and is given in real pixels
    WorldFixedSize,

    // Is positioned in the game world and has a size in world units
    World
  };

  // Explicitly initialize shape
  Canvas(GameObject &associatedObject, Space space, Vector2 size = Vector2::Zero());

  virtual ~Canvas();

  void OnBeforeDestroy() override;

  // Cascade these hooks down the canvas object tree
  void CascadeDown(std::function<void(GameObject &)> callback, bool topDown = true) override;

  // Add a UI Object to the canvas object tree as child of root canvas object
  template <class T, typename... Args>
  std::shared_ptr<T> AddChild(std::string objectName, Args &&...args)
  {
    return InsertInto<T>(nullptr, objectName, std::forward<Args>(args)...);
  }

  // Converts from real pixels relative to canvas top-left to world units
  Vector2 CanvasToWorld(Vector2 position, std::shared_ptr<Camera> camera = Camera::GetMain()) const;

  // Converts from world units to real pixels relative to canvas top-left
  Vector2 WorldToCanvas(Vector2 position, std::shared_ptr<Camera> camera = Camera::GetMain()) const;

  // Converts from real pixels relative to canvas top-left to
  Vector2 CanvasToScreen(Vector2 position, std::shared_ptr<Camera> camera = Camera::GetMain()) const;

  // Converts from real pixels relative to camera's top left to real pixels relative to canvas top-left
  Vector2 ScreenToCanvas(Vector2 position, std::shared_ptr<Camera> camera = Camera::GetMain()) const;

  // How the canvas is related to the world
  Space space;

  // Size of the canvas
  // Ignored when Global space, in real pixels when WorldFixedSize space, and in world units when World space
  Vector2 size;

  // The root UI Object
  std::shared_ptr<UIContainer> root;

  RenderLayer GetRenderLayer() override { return RenderLayer::Debug; }
  void Render() override;

private:
  // Get top-left position of the canvas in world units
  Vector2 GetTopLeft() const;

  // Initialize the inheritable properties of the root object
  void InitializeRootStyle();

  // Add a UI Object to the canvas object tree, as a child of the given UI Object (nullptr for child of canvas root)
  template <class T, typename... Args>
  std::shared_ptr<T> InsertInto(std::shared_ptr<UIContainer> parent, std::string objectName, Args &&...args)
  {
    // Create the UI Object
    auto newObject = GetScene()->NewObject<T>(*this, objectName, parent, std::forward<Args>(args)...);

    // Initialize it's dimensions
    newObject->InitializeDimensions();
    
    return newObject;
  }
};

#include "UIContainer.h"

#endif