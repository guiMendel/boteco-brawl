#ifndef __UI_OBJECT__
#define __UI_OBJECT__

#include "GameObject.h"
#include "Renderable.h"
#include "UIDimension.h"

class GameScene;
class UIContainer;
class Canvas;

// A specific kind of GameObject which exists in the ui dimension, has 2D dimensions, has a style, and reacts to user input events
class UIObject : public GameObject, public Renderable
{
  friend class GameScene;
  friend class UIDimension;

protected:
  // Constructor dedicated for a scene's root object
  // Initialize with given scene
  UIObject(Canvas &canvas, std::string name, int gameSceneId, int id = -1);

public:
  // With properties
  UIObject(Canvas &canvas, std::string name, std::shared_ptr<UIContainer> parent = nullptr);

  virtual ~UIObject();

  // =================================
  // OBJECT PROPERTIES
  // =================================
public:
  // Returns the real pixel displacement between the top-left of the Canvas and this object's anchor point
  Vector2 GetPosition() override;

  // Width of the object
  UIDimension width;

  // Height of the object
  UIDimension height;

  // Minimum space between this object's edges and it's content box
  DirectedDimension padding;

  // Minimum space between this object's edges and anything else on the outside
  DirectedDimension margin;

private:
  // Gets either width or height, depending on the provided axis
  UIDimension &GetSize(UIDimension::Axis axis);

  // Last calculate value of position in real pixels
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
  // UTILITY
  // =================================
public:
  // Easy access to the canvas this object is associated to
  Canvas &canvas;

private:
  std::shared_ptr<UIObject> GetShared();
};

#include "GameScene.h"
#include "Canvas.h"

#endif