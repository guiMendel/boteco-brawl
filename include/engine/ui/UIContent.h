#ifndef __UI_CONTENT__
#define __UI_CONTENT__

#include "UIObject.h"

class UIContainer;

// A UI Object which can contain children UI Objects
class UIContent : public UIObject
{
  friend class GameScene;
  friend class UIDimension;

public:
  UIContent(Canvas &canvas, std::string name, std::shared_ptr<UIContainer> parent);

  virtual ~UIContent() {}

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
  // Executes the given function for this object and then cascades it down to any children it has
  void CascadeDown(std::function<void(GameObject &)> callback, bool topDown = true) override;

protected:
  std::shared_ptr<GameObject> InternalGetParentNoException() const override;
};

#include "UIInheritable.h"

#endif