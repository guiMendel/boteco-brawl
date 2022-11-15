#ifndef __GAME_DATA__
#define __GAME_DATA__

class GameData
{
public:
  static GameData &GetInstance()
  {
    static GameData instance;

    return instance;
  }

  bool playerWon{false};
};

#endif