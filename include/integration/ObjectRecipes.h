#ifndef __OBJECT_RECIPES__
#define __OBJECT_RECIPES__

#include <memory>
#include "GameObject.h"
#include "Player.h"
#include "Text.h"

#define DASH_PARTICLES_OBJECT "DashParticles"

class ObjectRecipes
{
public:
  static auto Camera(float size = 5) -> std::function<void(std::shared_ptr<GameObject>)>;

  static auto Background(std::string imagePath) -> std::function<void(std::shared_ptr<GameObject>)>;

  static auto Character(std::shared_ptr<Player> player) -> std::function<void(std::shared_ptr<GameObject>)>;

  static auto PlayerManager() -> std::function<void(std::shared_ptr<GameObject>)>;

  static auto Platform(Vector2 size, bool isStatic = true) -> std::function<void(std::shared_ptr<GameObject>)>;
};

#endif