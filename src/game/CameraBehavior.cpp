#include "CameraBehavior.h"
#include "Debug.h"

using namespace std;

CameraBehavior::CameraBehavior(GameObject &associatedObject, shared_ptr<GameObject> charactersParent)
    : Component(associatedObject), weakCharactersParent(charactersParent) {}

void CameraBehavior::Awake()
{
  weakCamera = gameObject.RequireComponent<Camera>();
  auto arena = GetState()->FindObjectOfType<Arena>();
  weakArena = arena;

  arenaAspectRatio = arena->width / arena->height;
}

void CameraBehavior::Start()
{
  Reset();
}

void CameraBehavior::Update(float)
{
  LOCK(weakCharactersParent, charactersParent);

  // Get the characters
  auto characters = charactersParent->GetChildren();
}

void CameraBehavior::Reset()
{
  LOCK(weakCamera, camera);
  LOCK(weakArena, arena);

  // Aspect ratio of game screen
  static const float screenRatio = Game::screenWidth / Game::screenHeight;

  // Adjust size to be the maximum possible which still fits inside the arena
  if (arenaAspectRatio >= screenRatio)
    camera->SetSize(arena->height / 2);

  else
    camera->SetSize(arena->width / screenRatio / 2);

  // Reset position
  gameObject.SetPosition({0, 0});
}
