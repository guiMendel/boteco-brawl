#ifndef __RECIPES__
#define __RECIPES__

#include <memory>
#include "GameObject.h"
#include "Alien.h"
#include "Text.h"

class Recipes
{
public:
  // Map
  static void Tilemap(std::shared_ptr<GameObject> tilemap);

  // Penguins
  static void PenguinBody(std::shared_ptr<GameObject> penguin);
  static void PenguinCannon(std::shared_ptr<GameObject> penguin);

  // Aliens
  static void Alien(std::shared_ptr<GameObject> alien);
  static auto Minion(std::shared_ptr<::Alien> alien, float startingArc) -> std::function<void(std::shared_ptr<GameObject>)>;

  // General
  static auto Text(std::string text, int size = 10, Color color = Color::White(), Text::Style style = Text::Style::solid) -> std::function<void(std::shared_ptr<GameObject>)>;
  static auto Background(std::string imagePath) -> std::function<void(std::shared_ptr<GameObject>)>;
  static auto OneShotAnimation(std::string spritePath, Vector2 animationFrame, float animationSpeed) -> std::function<void(std::shared_ptr<GameObject>)>;

  // Projectile
  static auto Projectile(std::string spritePath, Vector2 animationFrame, float animationSpeed, bool loopAnimation,
                         Tag targetTag,
                         float startingAngle,
                         float speed = 300.0f,
                         float timeToLive = 5.0f,
                         float damage = 50.0f,
                         std::weak_ptr<GameObject> target = std::weak_ptr<GameObject>(),
                         float chaseSteering = 0.5f)
      -> std::function<void(std::shared_ptr<GameObject>)>;
};

#endif