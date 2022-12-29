#ifndef __MAIN_STATE__
#define __MAIN_STATE__

#include "GameState.h"
#include "GameObject.h"
#include <functional>
#include <memory>

class MainState : public GameState
{
public:
  virtual ~MainState() {}

  void LoadAssets() override;
  void InitializeObjects() override;
  void OnUpdate(float) override;

private:
  Music music;
};

#endif