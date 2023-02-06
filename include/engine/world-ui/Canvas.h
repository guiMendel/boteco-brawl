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

  // Raised when the canvas size changes
  // Provides new and old values of size
  EventII<Vector2, Vector2> OnChangeSize;

  Canvas(GameObject &associatedObject,
         Space space,
         Vector2 size = Vector2::Zero(),
         std::shared_ptr<Camera> camera = Camera::GetMain());

  RenderLayer GetRenderLayer() override { return RenderLayer::Debug; }
  void Render() override;

  virtual ~Canvas();

  // Get the canvas size (in units or real pixels, depending on it's current space)
  Vector2 GetSize() const;

  // Set the canvas size (in units or real pixels, depending on it's current space)
  void SetSize(Vector2 newSize);

  // Get the current canvas space
  Space GetSpace() const;

  // Set the canvas space
  void SetSpace(Space newSpace);

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

  // Defines the point inside the canvas that will be displayed at it's position
  // Ignored on global canvas type
  // Values be in range [0, 1]
  Vector2 anchorPoint{0.5, 0.5};

  // The root UI Object
  std::shared_ptr<UIContainer> root;

  // To which camera this canvas is associated
  std::weak_ptr<Camera> weakCamera;

private:
  // Get position of the anchor point in world units
  Vector2 GetAnchorPosition() const;

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