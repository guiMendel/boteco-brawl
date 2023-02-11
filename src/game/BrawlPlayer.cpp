#include "BrawlPlayer.h"

using namespace std;

BrawlPlayer::BrawlPlayer(
    GameObject &associatedObject,
    PlayerManager &manager,
    Color color,
    string hoverBadgePath,
    string selectionBadgePath)
    : Player(associatedObject, manager, color),
      hoverBadgePath(hoverBadgePath),
      selectionBadgePath(selectionBadgePath) {}
