#ifndef __TITLE_STATE__
#define __TITLE_STATE__

#include "GameState.h"

class TitleState : public GameState
{
public:
  virtual ~TitleState() {}

  // How long the instructions takes to switch between visible and non visible
  static const float instructionFlashTime;

  void InitializeObjects() override;

  void Update(float deltaTime) override;

private:
  std::weak_ptr<GameObject> instructionWeak;
};

#endif
