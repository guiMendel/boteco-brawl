#ifndef __UI_CONTAINER_
#define __UI_CONTAINER_

#include "UIObject.h"
#include "Parent.h"
#include "UIChildrenBox.h"

// Defines the properties of a flexbox container
struct UIFlexboxProperties
{
  // Gets a hash to identify this configuration set
  int GetHash() const;

  // Sets dimensions owners
  void SetOwner(std::shared_ptr<UIObject> owner);

  // Defines the main axis of the flexbox
  UIDimension::Axis mainAxis{UIDimension::Horizontal};

  // Minimum empty space between container's items
  UIDimension2 gap;

  // Whether to wrap items if they reach the container's limits
  bool wrap{false};

  // Where to align items to within the container's available space
  // Values must be in range [0, 1]
  Vector2 placeItems;

  // Whether to revert render direction within the box's groups
  bool reverseDirection{false};
};

// A UI Object which can contain children UI Objects
class UIContainer : public UIObject, public Parent<UIObject>
{
  friend class GameScene;
  friend class UIDimension;
  friend class Canvas;
  friend class UIChildrenBox;
  friend class UIObject;

public:
  UIContainer(Canvas &canvas,
              std::string name,
              std::shared_ptr<UIContainer> parent,
              UIFlexboxProperties properties = UIFlexboxProperties());

  virtual ~UIContainer() {}

  void Awake() override;
  void Update(float) override;

  // =================================
  // OBJECT PROPERTIES
  // =================================
public:
  // Is the size of the children box
  int GetContentRealPixelsAlong(
      UIDimension::Axis axis,
      UIDimension::Calculation config = UIDimension::Calculation::Default) override;

  // Calculates the size of the children box treating those who depend on this container's size as 0
  // Returns horizontal size first
  int GetIndependentContentRealPixels(UIDimension::Axis axis, UIDimension::Calculation config) const;

private:
  void InitializeDimensions() override;

  void PrecalculateDimensions() override;

  // =================================
  // UI EVENTS
  // =================================
protected:
  // Checks if the given OnMouseOver event applies to this object
  // If it does, raises the event and propagates the check downwards
  void CheckEventApplication(std::shared_ptr<UIMouseEvent> onMouseOverEvent) override;

  // =================================
  // OBJECTS HIERARCHY
  // =================================
public:
  // Add a UI Object to the canvas object tree as child of this container
  template <class T, typename... Args>
  std::shared_ptr<T> AddChild(std::string objectName, Args &&...args)
  {
    return canvas.InsertInto<T>(GetShared(), objectName, std::forward<Args>(args)...);
  }

  // Executes the given function for this object and then cascades it down to any children it has
  void CascadeDown(std::function<void(GameObject &)> callback, bool topDown = true) override;

  // Adds arrangement order sorting
  std::vector<std::shared_ptr<UIObject>> GetChildren() override;

protected:
  std::shared_ptr<GameObject> InternalGetParentNoException() const override;

  // Generator for arrangement order of children
  int arrangeOrderGenerator{0};

  // =================================
  // CHILDREN BOX
  // =================================
public:
  // Allows access to the current flexbox properties of this container
  UIFlexboxProperties &Flexbox();

protected:
  // Detects whether children box recalculation is necessary and performs it if so
  void DetectRecalculation();

  // Recalculates the children box
  void RecalculateChildrenBox();

  // Flexbox properties for the container
  UIFlexboxProperties properties;

  // Currently calculated children box
  UIChildrenBox childrenBox;

  // Last hash value of properties
  int lastPropertiesHash;

  // Whether should check if the properties have changed since last frame
  bool checkPropertyChange{false};

  // Force children box recalculation in next update frame
  bool forceRecalculation{false};

  // =================================
  // DESTRUCTION
  // =================================
protected:
  // Calls DestroySelf and ignores returned value
  void InternalDestroy() override;

private:
  // Destroys children, calls GameObject::InternalDestroy, then unlinks from parent
  // Returns a valid iterator for the parent's new children after unlinking
  auto DestroySelf() -> std::unordered_map<int, std::weak_ptr<UIObject>>::iterator override;

  // =================================
  // UTILITY
  // =================================
public:
  std::shared_ptr<UIContainer> GetShared() const;
};

#include "UIInheritable.h"

#endif