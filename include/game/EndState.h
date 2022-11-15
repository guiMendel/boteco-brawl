#ifndef __END_STATE__
#define __END_STATE__

#include "GameState.h"
#include "Music.h"

class EndState : public GameState
{
public:
  virtual ~EndState() {}

  void InitializeObjects() override;

  void Update(float deltaTime) override;

  Music music;
};

#endif
