#ifndef __MAIN_SCENE__
#define __MAIN_SCENE__

#include "GameScene.h"
#include "WorldObject.h"
#include <functional>
#include <memory>

#define MAIN_PARENT_OBJECT "MainParent"
#define CHARACTERS_PARENT "CharactersParent"
#define CURTAIN_OBJECT "Curtain"
#define COUNTDOWN_OBJECT "CountdownContainer"
#define VICTORY_TEXT "VictoryText"

class ArenaScene : public GameScene
{
public:
  virtual ~ArenaScene() {}

  std::string GetName() const override;

  void InitializeObjects() override;
  void OnUpdate(float) override;

  Music music;
  Music background;

private:
  void SpawnCharacters();

  void SetupArena();
};

#endif