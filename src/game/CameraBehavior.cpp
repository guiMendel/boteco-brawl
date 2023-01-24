#include "CameraBehavior.h"
#include "FallOffDeath.h"
#include "BoxCollider.h"
#include "Debug.h"

using namespace std;

const float CameraBehavior::padding{1.5};

// Aspect ratio of game screen
static const float screenRatio = Game::screenWidth / Game::screenHeight;

CameraBehavior::CameraBehavior(GameObject &associatedObject, shared_ptr<GameObject> charactersParent)
    : Component(associatedObject), weakCharactersParent(charactersParent) {}

void CameraBehavior::Awake()
{
  weakCamera = gameObject.RequireComponent<Camera>();
  auto arena = GetState()->FindObjectOfType<Arena>();
  weakArena = arena;

  arenaAspectRatio = arena->width / arena->height;

  maxSize = min(arena->height / 2, arena->width / screenRatio / 2);
}

void CameraBehavior::Start()
{
  Reset();
}

void CameraBehavior::Update(float)
{
  LOCK(weakCharactersParent, charactersParent);

  // Will hold the center point between all characters
  Vector2 centerPoint;

  int liveCharacters{0};

  for (auto character : charactersParent->GetChildren())
  {
    // Filter out dead ones
    if (character->RequireComponent<FallOffDeath>()->IsDead())
      continue;

    liveCharacters++;
    centerPoint += character->GetPosition();
  }

  if (liveCharacters > 0)
    centerPoint /= liveCharacters;

  // Move to center point
  MoveTo(centerPoint);
}

void CameraBehavior::MoveTo(Vector2 position)
{
  // Set the new position
  gameObject.SetPosition(position);

  // Frame characters
  FitCharacters();
}

void CameraBehavior::FitCharacters()
{
  LOCK(weakCharactersParent, charactersParent);

  // Maximum distance from a character's outer side from the center of the camera
  Vector2 maxDistances;

  for (auto character : charactersParent->GetChildren())
  {
    // Get it's box
    auto characterBox = character->RequireComponent<BoxCollider>()->GetBox();

    // Keep biggest x distance
    maxDistances.x = max(
        maxDistances.x,
        abs(character->GetPosition().x - gameObject.GetPosition().x) + characterBox.width / 2);

    // Keep biggest y distance
    maxDistances.y = max(
        maxDistances.y,
        abs(character->GetPosition().y - gameObject.GetPosition().y) + characterBox.height / 2);
  }

  LOCK(weakCamera, camera);


  // Set the size to the largest dimension
  float newSize = max(maxDistances.y, maxDistances.x / screenRatio) + padding;
  
  // Ensure it doesn't surpass max size possible for arena size
  camera->SetSize(min(newSize, maxSize));

  float horizontalSize = camera->GetSize() * screenRatio;

  LOCK(weakArena, arena);

  // Adjust position to fit within arena
  gameObject.SetPosition(Vector2(
    Clamp(gameObject.GetPosition().x, -arena->width / 2 + horizontalSize, arena->width / 2 - horizontalSize),
    Clamp(gameObject.GetPosition().y, -arena->height / 2 + camera->GetSize(), arena->height / 2 - camera->GetSize())
  ));
}

void CameraBehavior::Reset()
{
  LOCK(weakCamera, camera);
  LOCK(weakArena, arena);

  gameObject.SetPosition({0, 0});

  // Adjust size to be the maximum possible which still fits inside the arena
  camera->SetSize(maxSize);
}
