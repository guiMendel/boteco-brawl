#ifndef __MAIN_STATE__
#define __MAIN_STATE__

#include "GameState.h"
#include "GameObject.h"
#include "Tilemap.h"
#include <functional>
#include <memory>

class MainState : public GameState
{
public:
  virtual ~MainState() {}

  void InitializeObjects() override;

private:
  Music music;
};

#endif