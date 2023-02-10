#ifndef __MENU_SCENE__
#define __MENU_SCENE__

#include "GameScene.h"

class MenuScene : public GameScene
{
public:
  virtual ~MenuScene() {}

  void InitializeObjects() override;

private:
  // Create the splash menu
  void CreateSplash(std::shared_ptr<UIContainer> mainContainer);

  // Create the selection menu
  void CreateSelection(std::shared_ptr<UIContainer> mainContainer);
};

#endif