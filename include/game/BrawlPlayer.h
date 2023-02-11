#ifndef __BRAWL_PLAYER__
#define __BRAWL_PLAYER__

#include "Player.h"

class BrawlPlayer : public Player
{
public:
  BrawlPlayer(
      GameObject &associatedObject,
      PlayerManager &manager,
      Color color,
      std::string hoverBadgePath,
      std::string selectionBadgePath);

  // The UI selection hover badge image path
  std::string hoverBadgePath;

  // The UI selection badge image path
  std::string selectionBadgePath;
};

#endif