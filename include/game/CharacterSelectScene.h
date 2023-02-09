#ifndef __CHARACTER_SELECT_SCENE__
#define __CHARACTER_SELECT_SCENE__

#include "GameScene.h"

class CharacterSelectScene : public GameScene
{
public:
  virtual ~CharacterSelectScene() {}

  void InitializeObjects() override;

private:
  Music music;
};

#endif