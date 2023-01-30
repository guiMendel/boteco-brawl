#ifndef __UI_ROOT_
#define __UI_ROOT_

#include "UIContainer.h"

// A UI Object which can contain children UI Objects
class UIRoot : public UIContainer
{
  friend class GameScene;
  friend class UIDimension;

protected:
  // Constructor dedicated for a scene's root object
  // Initialize with given scene
  UIRoot(Canvas &canvas, int gameSceneId, int id = -1);

  std::shared_ptr<GameObject> InternalGetParent() const override;

public:
  // Name of the root object
  static const std::string rootName;

  // With properties
  UIRoot(Canvas &canvas);
};

#endif