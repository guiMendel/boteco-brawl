#ifndef __BRAWL_PLAYER__
#define __BRAWL_PLAYER__

#include "Player.h"
#include "ObjectRecipes.h"

class BrawlPlayer : public Player
{
public:
  BrawlPlayer(
      GameObject &associatedObject,
      PlayerManager &manager,
      Color color,
      std::string hoverBadgePath,
      std::string selectionBadgePath);

  // Set a character for this player
  template <class T>
  void SetCharacter()
  {
    characterRecipe = ObjectRecipes::Character<T>(GetShared());
  }

  // The UI selection hover badge image path
  std::string hoverBadgePath;

  // The UI selection badge image path
  std::string selectionBadgePath;

  // Recipe to create this player's character
  ObjectRecipes::Recipe characterRecipe;
};

#endif