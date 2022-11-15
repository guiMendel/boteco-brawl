#ifndef __MAIN_STATE__
#define __MAIN_STATE__

#include "GameState.h"
#include "GameObject.h"
#include "Alien.h"
#include "Tilemap.h"
#include <functional>
#include <memory>

class MainState : public GameState
{
public:
  virtual ~MainState() {}

  // How many aliens to start with
  static const int totalAliens;

  // How long to stop state after player dies
  static const float dieAdvanceTime;

  // How much slack the penguin ahs on the edges before dying
  static const float edgeSlack;

  void InitializeObjects() override;

  void Update(float deltaTime) override;

  void AdvanceState(bool victory);

private:
  Music music;

  std::weak_ptr<GameObject> penguinWeak;
  std::weak_ptr<TileMap> tilemapWeak;

  // How many aliens there still are
  int alienCount{totalAliens};
};

#endif