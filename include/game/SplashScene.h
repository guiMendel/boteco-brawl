#ifndef __SPLASH_SCENE__
#define __SPLASH_SCENE__

#include "GameScene.h"

class SplashScene : public GameScene
{
public:
  virtual ~SplashScene() {}

  void InitializeObjects() override;

private:
  Music music;
};

#endif