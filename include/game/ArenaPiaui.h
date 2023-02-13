#ifndef __ARENA_PIAUI__
#define __ARENA_PIAUI__

#include "Arena.h"

class ArenaPiaui : public Arena
{
public:
  ArenaPiaui(GameObject &associatedObject);

protected:
  // Set up the ArenaPiaui space
  void InitializeArena() override;
};

#endif