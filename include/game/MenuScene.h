#ifndef __MENU_SCENE__
#define __MENU_SCENE__

#include "GameScene.h"

#define MAIN_CONTAINER_OBJECT "Main"
#define CURTAIN_OBJECT "Curtain"
#define SPLASH_OBJECT "Splash"
#define SUBTITLE_OBJECT "Subtitle"
#define START_PROMPT_OBJECT "StartPrompt"
#define PARTICLES_OBJECT "StompParticles"
#define BILLS_OBJECT "PlayerSelections"
#define OPTIONS_OBJECT "CharacterOptions"
#define BACK_BUTTON_IMAGE "BackButton"
#define BILL_IMAGE "Bill"
#define IDLE_ANIMATIONS_OBJECT "IdleAnimations"
#define START_ARENA_IMAGE "StartArenaPrompt"

class MenuScene : public GameScene
{
public:
  virtual ~MenuScene() {}

  std::string GetName() const override;

  void InitializeObjects() override;

private:
  // Create the splash menu
  void CreateSplash(std::shared_ptr<UIContainer> mainContainer);

  // Create the selection menu
  void CreateSelection(std::shared_ptr<UIContainer> mainContainer);
};

#endif