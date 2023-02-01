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
  // With parent
  UIContent(Canvas &canvas, std::shared_ptr<UIContainer> parent, std::string name);

  // Without parent
  UIContent(Canvas &canvas, std::string name);

  virtual ~UIContent() {}

  // =================================
  // OBJECTS HIERARCHY
  // =================================
public:
  // Executes the given function for this object and then cascades it down to any children it has
  void CascadeDown(std::function<void(GameObject &)> callback, bool topDown = true) override;

protected:
  std::shared_ptr<GameObject> InternalGetParent() const override;
};

#include "UIInheritable.h"

#endif