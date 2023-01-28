#ifndef __MAIN_SCENE__
#define __MAIN_SCENE__

#include "GameScene.h"
#include "WorldObject.h"
#include <functional>
#include <memory>

class MainScene : public GameScene
{
public:
  virtual ~MainScene() {}

  void LoadAssets() override;
  void InitializeObjects() override;
  void OnUpdate(float) override;

private:
  Music music;
};

#endif