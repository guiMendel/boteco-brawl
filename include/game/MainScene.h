#ifndef __MAIN_SCENE__
#define __MAIN_SCENE__

#include "GameScene.h"
#include "WorldObject.h"
#include <functional>
#include <memory>

#define CHARACTERS_PARENT "CharactersParent"
#define CURTAIN_OBJECT "Curtain"

class MainScene : public GameScene
{
public:
  virtual ~MainScene() {}

  void InitializeObjects() override;
  void OnUpdate(float) override;

private:
  void SpawnCharacters();

  void SetupArena();

  Music music;
};

#endif