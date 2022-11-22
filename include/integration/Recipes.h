#ifndef __RECIPES__
#define __RECIPES__

#include <memory>
#include "GameObject.h"
#include "Text.h"

class Recipes
{
public:
  static auto Camera(float size = 5) -> std::function<void(std::shared_ptr<GameObject>)>;

  static auto Background(std::string imagePath) -> std::function<void(std::shared_ptr<GameObject>)>;

  static auto Character() -> std::function<void(std::shared_ptr<GameObject>)>;

  static auto Platform(Vector2 size, bool isStatic = true) -> std::function<void(std::shared_ptr<GameObject>)>;
};

#endif