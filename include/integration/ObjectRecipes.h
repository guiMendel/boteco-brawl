#ifndef __OBJECT_RECIPES__
#define __OBJECT_RECIPES__

#include <memory>
#include "GameObject.h"
#include "Player.h"
#include "Text.h"

#define DASH_PARTICLES_OBJECT "DashParticles"
#define CHARACTER_SLIDE_BOX_OBJECT "RepelBox"
#define CHARACTER_PLATFORM_DROP_OBJECT "PlatformDropDetector"
#define HITBOX_OBJECT "Hitbox"

class ObjectRecipes
{
public:
  template <class T>
  static auto SingleComponent(bool dontDestroyOnLoad = false) -> std::function<void(std::shared_ptr<GameObject>)>
  {
    return [dontDestroyOnLoad](std::shared_ptr<GameObject> object)
    {
      object->AddComponent<T>();
      if (dontDestroyOnLoad)
        object->DontDestroyOnLoad();
    };
  }

  static auto Camera(float size = 5) -> std::function<void(std::shared_ptr<GameObject>)>;

  static auto Arena(std::string imagePath) -> std::function<void(std::shared_ptr<GameObject>)>;

  static auto CharacterStateManager(std::shared_ptr<Player> player) -> std::function<void(std::shared_ptr<GameObject>)>;

  static auto Platform(Vector2 size, bool withEffector = false) -> std::function<void(std::shared_ptr<GameObject>)>;

  static auto Projectile(Vector2 initialSpeed, std::shared_ptr<GameObject> parent) -> std::function<void(std::shared_ptr<GameObject>)>;

};

#endif