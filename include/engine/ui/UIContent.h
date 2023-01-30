#ifndef __UI_CONTENT__
#define __UI_CONTENT__

#include "UIObject.h"

class UIContainer;

// A UI Object which can contain children UI Objects
class UIContent : public UIObject
{
  friend class GameScene;
  friend class UIDimension;

protected:
  // Constructor dedicated for a scene's root object
  // Initialize with given scene
  UIContent(Canvas &canvas, std::string name, int gameSceneId, int id = -1);

public:
  // With properties
  UIContent(Canvas &canvas, std::string name, std::shared_ptr<UIContainer> parent = nullptr);

  // =================================
  // OBJECTS HIERARCHY
  // =================================
public:
  // Executes the given function for this object and then cascades it down to any children it has
  void CascadeDown(std::function<void(GameObject &)> callback, bool topDown = true) override;
};

#endif