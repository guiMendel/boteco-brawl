#ifndef __UI_CONTAINER_
#define __UI_CONTAINER_

#include "UIObject.h"
#include "Parent.h"

// A UI Object which can contain children UI Objects
class UIContainer : public UIObject, public Parent<UIObject>
{
  friend class GameScene;
  friend class UIDimension;

protected:
  // Constructor dedicated for a scene's root object
  // Initialize with given scene
  UIContainer(Canvas &canvas, std::string name, int gameSceneId, int id = -1);

public:
  // With properties
  UIContainer(Canvas &canvas, std::string name, std::shared_ptr<UIContainer> parent = nullptr);

  // =================================
  // OBJECTS HIERARCHY
  // =================================
public:
  // Executes the given function for this object and then cascades it down to any children it has
  void CascadeDown(std::function<void(GameObject &)> callback, bool topDown = true) override;

protected:
  std::shared_ptr<GameObject> InternalGetParent() const override;

  // =================================
  // RENDERING
  // =================================
protected:
  void RegisterLayer() override;
};

#endif