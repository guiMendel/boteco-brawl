#ifndef __OBJECT_RECIPES__
#define __OBJECT_RECIPES__

#include <memory>
#include "WorldObject.h"
#include "Player.h"
#include "UIText.h"
#include "Character.h"

#define CHARACTER_SLIDE_BOX_OBJECT "RepelBox"
#define CHARACTER_PLATFORM_DROP_OBJECT "PlatformDropDetector"
#define CHARACTER_UI_OBJECT "CharacterUI"
#define CHARACTER_LIFE_OBJECT "LifeIcons"

class ObjectRecipes
{
public:
  // Defines a recipe type
  using Recipe = std::function<void(std::shared_ptr<WorldObject>)>;

  template <class T>
  static auto SingleComponent(bool dontDestroyOnLoad = false) -> Recipe
  {
    return [dontDestroyOnLoad](std::shared_ptr<WorldObject> object)
    {
      object->AddComponent<T>();
      if (dontDestroyOnLoad)
        object->DontDestroyOnLoad();
    };
  }

  static auto Camera() -> Recipe;

  template <class T>
  static auto Character(std::shared_ptr<Player> player) -> Recipe
  {
    return [weakPlayer = std::weak_ptr(player)](std::shared_ptr<WorldObject> object)
    {
      // Give it the character component
      auto selectedCharacter = object->AddComponent<T>();

      // Initialize it
      InitializeCharacter(selectedCharacter, Helper::Lock(weakPlayer));
    };
  }

  static auto Canvas(Canvas::Space space) -> Recipe;

  static auto Platform(Vector2 size, bool withEffector = false) -> Recipe;

  static auto Projectile(Vector2 initialSpeed, std::shared_ptr<WorldObject> parent, Vector2 gravityScale = Vector2::One())
      -> Recipe;

private:
  // Initializes a character's object once it already has it's character component
  static void InitializeCharacter(std::shared_ptr<::Character> object, std::shared_ptr<Player> player);
};

#endif