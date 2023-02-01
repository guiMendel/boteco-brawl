#ifndef __UI_CONTAINER_
#define __UI_CONTAINER_

#include "UIObject.h"
#include "Parent.h"

// A UI Object which can contain children UI Objects
class UIContainer : public UIObject, public Parent<UIObject>
{
  friend class GameScene;
  friend class UIDimension;

public:
  // With properties
  UIContainer(Canvas &canvas, std::shared_ptr<UIContainer> parent, std::string name);

  // Without parent
  UIContainer(Canvas &canvas, std::string name);

  virtual ~UIContainer() {}

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

protected:
  std::shared_ptr<GameObject> InternalGetParent() const override;

  // =================================
  // UTILITY
  // =================================
public:
  std::shared_ptr<UIContainer> GetShared();
};

#include "UIInheritable.h"

#endif