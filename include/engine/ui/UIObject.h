#ifndef __UI_OBJECT__
#define __UI_OBJECT__

#include "GameObject.h"
#include "Renderable.h"
#include "UIDimension.h"

class GameScene;
class UIContainer;
class Canvas;
class UIInheritable;

// A specific kind of GameObject which exists in the ui dimension, has 2D dimensions, has a style, and reacts to user input events
class UIObject : public GameObject, public Renderable
{
  friend class GameScene;
  friend class UIDimension;
  friend class UIContainer;

public:
  // Will be disconnected from canvas object tree until manually inserted as child of another object
  UIObject(Canvas &canvas, std::string name);

  virtual ~UIObject();

  // =================================
  // OBJECT PROPERTIES
  // =================================
public:
  // Returns the real pixel displacement between the top-left of the Canvas and this object's top-left
  Vector2 GetPosition() override;

  // Width of the object
  UIDimension width;

  // Height of the object
  UIDimension height;

  // Minimum space between this object's edges and it's content box
  UIDirectedDimension padding;

  // Minimum space between this object's edges and anything else on the outside
  UIDirectedDimension margin;

  // Defines all properties of the object that can be inherited from it's parent
  std::unique_ptr<UIInheritable> style;

private:
  // Gets either width or height, depending on the provided axis
  UIDimension &GetSize(UIDimension::Axis axis);

  // Last calculated value of position in real pixels
  Vector2 updatedPosition;

  // =================================
  // OBJECTS HIERARCHY
  // =================================
public:
  // Whether this object is the canvas root
  bool IsCanvasRoot() const;

  // Gets pointer to parent, throws if it's invalid or if this is the root object
  std::shared_ptr<UIContainer> GetParent() const;

  // Set the parent
  void SetParent(std::shared_ptr<UIContainer> newParent);

  // Check if this uiObject is in the descendant lineage of the other object
  bool IsDescendantOf(std::shared_ptr<UIContainer> other) const;

  // Check if either object is a descendent of each other
  static bool SameLineage(
      std::shared_ptr<UIObject> first, std::shared_ptr<UIObject> second);

private:
  // Parent object
  std::weak_ptr<UIContainer> weakParent;

  // =================================
  // DESTRUCTION
  // =================================
protected:
  // Calls DestroySelf and ignores returned value
  void InternalDestroy() override;

  // Destroys children, calls GameObject::InternalDestroy, then unlinks from parent
  // Returns a valid iterator for the parent's new children after unlinking
  virtual auto DestroySelf() -> std::unordered_map<int, std::weak_ptr<UIObject>>::iterator;

  // Destroys relation to parent (becomes unassociated to the canvas object tree)
  auto UnlinkParent() -> std::unordered_map<int, std::weak_ptr<UIObject>>::iterator;

  // =================================
  // UTILITY
  // =================================
public:
  // Easy access to the canvas this object is associated to
  Canvas &canvas;

private:
  std::shared_ptr<UIObject> GetShared();

  // =================================
  // RENDERING
  // =================================
public:
  // Use value from style
  RenderLayer GetRenderLayer() final override;

  // Use value from style
  int GetRenderOrder() final override;

  void Render() override;

protected:
  void RegisterLayer() final override;

  void RegisterToScene() final override;
};

#include "GameScene.h"
#include "Canvas.h"

#endif