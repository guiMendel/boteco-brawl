#ifndef __UI_OBJECT__
#define __UI_OBJECT__

#include "GameObject.h"
#include "Renderable.h"
#include "UIDimension.h"
#include "UIEvent.h"

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
  friend class Canvas;

public:
  // Will be disconnected from canvas object tree until manually inserted as child of another object
  UIObject(Canvas &canvas, std::string name, std::shared_ptr<UIContainer> parent);

  void Awake() override;
  void OnBeforeDestroy() override;
  void Update(float) override;

  virtual ~UIObject();

  // =================================
  // OBJECT PROPERTIES
  // =================================
public:
  // Returns the real pixel displacement between the top-left of the canvas and this object's top-left
  Vector2 GetPosition() override;

  // Returns top-left position after counting in the padding
  Vector2 GetContentPosition();

  // Gets either width or height, depending on the provided axis
  UIDimension &GetDimension(UIDimension::Axis axis);

  // Gets real pixel size along an axis (size + padding [and possibly + margin])
  int GetRealPixelsAlong(UIDimension::Axis axis, bool includePadding = true, bool includeMargin = false);

  // Gets real pixel size along an axis of this object's content (whatever is inside of it)
  virtual int GetContentRealPixelsAlong(
      UIDimension::Axis axis,
      UIDimension::Calculation config = UIDimension::Calculation::Default) = 0;

  // Set local position, where the first value is along the given axis and the second is along the other axis
  void SetLocalPositionAlong(UIDimension::Axis axis, int mainSize, int crossSize);

  int GetUnpaddedWidth();

  int GetUnpaddedHeight();

  // Sets whether position is automatically calculated or given as an absolute value relative to parent
  void SetPositionAbsolute(bool value);

  // Checks if position is set to absolute
  bool IsPositionAbsolute() const;

  // Gets absolute scale of this object
  Vector2 GetScale() const;

  // Width of the object (padding + content)
  UIDimension width;

  // Height of the object (padding + content)
  UIDimension height;

  // Absolute coordinates of this object (relative to parent)
  // When none, anchors self based on parent's place items flex property
  UIDimension2 absolutePosition;

  // Minimum space between this object's edges and it's content box
  UIDimension4 padding;

  // Minimum space between this object's edges and anything else on the outside
  UIDimension4 margin;

  // Defines all properties of the object that can be inherited from it's parent
  std::unique_ptr<UIInheritable> style;

  // Offsets render position but has no effect over non-children objects' positions
  UIDimension2 offset;

  // Scale of this object (does NOT affect positioning of other objects)
  Vector2 localScale{1, 1};

protected:
  std::string callbackIdentifier;

private:
  // Gives all dimensions this object's shared pointer
  virtual void InitializeDimensions();

  // Precalculates the dimensions real pixel values for this frame
  virtual void PrecalculateDimensions();

  // Gets real pixels of absolute position for this object
  Vector2 GetAbsolutePosition();

  // The real pixel displacement between the top-left of the parent and this object's top-left
  Vector2 localPosition;

  // Positioning type of this object uses absolute coordinates (relative to parent) instead of automatic calculation
  bool positionAbsolute{false};

  // =================================
  // UI EVENTS
  // =================================
public:
  // Raised  whenever a UI Event is triggered for this object
  EventI<std::shared_ptr<UIEvent>> OnUIEvent;

protected:
  // Checks if the given mouse event applies to this object
  // If it does, raises the event and propagates the check downwards
  virtual void CheckEventApplication(std::shared_ptr<UIMouseEvent> mouseEvent) = 0;

  // Raises a mouse over event for this object
  // Also raises mouse enter if necessary
  void RaiseMouseEvent(std::shared_ptr<UIMouseEvent> onMouseOverEvent);

  // Detects if a screen position is inside this object's area
  bool Contains(Vector2 screenPosition);

private:
  // Whether a mouse over event was raised last frame
  bool mouseOverLastFrame{false};

  // =================================
  // OBJECTS HIERARCHY
  // =================================
public:
  // Whether this object is the canvas root
  bool IsCanvasRoot() const;

  // Gets pointer to parent, throws if it's invalid
  std::shared_ptr<UIContainer> RequireParent() const;

  // Gets pointer to parent
  std::shared_ptr<UIContainer> GetParent() const;

  // Set the parent
  void SetParent(std::shared_ptr<UIContainer> newParent);

  // Check if this uiObject is in the descendant lineage of the other object
  bool IsDescendantOf(std::shared_ptr<UIContainer> other) const;

  // Check if either object is a descendent of each other
  static bool SameLineage(
      std::shared_ptr<UIObject> first, std::shared_ptr<UIObject> second);

  // Arrangement order priority within parent container
  int arrangeOrder{0};

protected:
  void SetParent(std::shared_ptr<UIContainer> newParent, std::shared_ptr<UIObject> ownPointer);

  void InternalSetParent(std::shared_ptr<GameObject> newParent, std::shared_ptr<GameObject> ownPointer = nullptr) override;

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