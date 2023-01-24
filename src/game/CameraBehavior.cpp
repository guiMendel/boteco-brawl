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
  // Get camera
  auto camera = gameObject.RequireComponent<Camera>();
  weakCamera = camera;

  // Initialize targets
  targetSize = camera->GetSize();
  targetPosition = gameObject.GetPosition();

  // Get arena
  auto arena = GetState()->FindObjectOfType<Arena>();
  weakArena = arena;

  // Calculate arena dependent params
  arenaAspectRatio = arena->width / arena->height;

  maxSize = min(arena->height / 2, arena->width / screenRatio / 2);
}

void CameraBehavior::Start()
{
  Reset();
}

void CameraBehavior::Update(float)
{
  // Calculate new targets
  UpdateTargets();

  SetPosition(targetPosition);
  SetSize(targetSize);

  // Adjust values to ensure camera is valid
  ConfineToArena();
}

void CameraBehavior::UpdateTargets()
{
  LOCK(weakCharactersParent, charactersParent);

  // Reset target position
  // It will hold the center point between all characters
  targetPosition = Vector2::Zero();

  // Count live characters
  int liveCharacters{0};

  for (auto character : charactersParent->GetChildren())
  {
    // Filter out dead ones
    if (character->RequireComponent<FallOffDeath>()->IsDead())
      continue;

    liveCharacters++;
    targetPosition += character->GetPosition();
  }

  // Get average if there were at least one alive
  if (liveCharacters > 0)
    targetPosition /= liveCharacters;

  // === NEW SIZE CALCULATION

  // Maximum distance from a character's outer side from the target position
  Vector2 maxDistances;

  for (auto character : charactersParent->GetChildren())
  {
    // Get it's box
    auto characterBox = character->RequireComponent<BoxCollider>()->GetBox();

    // Keep biggest x distance
    maxDistances.x = max(
        maxDistances.x,
        abs(character->GetPosition().x - targetPosition.x) + characterBox.width / 2);

    // Keep biggest y distance
    maxDistances.y = max(
        maxDistances.y,
        abs(character->GetPosition().y - targetPosition.y) + characterBox.height / 2);
  }

  LOCK(weakCamera, camera);

  // Set the size to the largest dimension
  targetSize = max(maxDistances.y, maxDistances.x / screenRatio) + padding;
}

void CameraBehavior::SetPosition(Vector2 position)
{
  // Set the new position
  gameObject.SetPosition(position);
}

void CameraBehavior::SetSize(float newSize)
{
  LOCK(weakCamera, camera);

  // Ensure it doesn't surpass max size possible for arena size
  camera->SetSize(min(newSize, maxSize));
}

void CameraBehavior::ConfineToArena()
{
  LOCK(weakArena, arena);
  LOCK(weakCamera, camera);

  float horizontalSize = camera->GetSize() * screenRatio;

  // Adjust position to fit within arena
  SetPosition(Vector2(
      Clamp(gameObject.GetPosition().x, -arena->width / 2 + horizontalSize, arena->width / 2 - horizontalSize),
      Clamp(gameObject.GetPosition().y, -arena->height / 2 + camera->GetSize(), arena->height / 2 - camera->GetSize())));
}

void CameraBehavior::Reset()
{
  LOCK(weakCamera, camera);
  LOCK(weakArena, arena);

  SetPosition({0, 0});

  // Adjust size to be the maximum possible which still fits inside the arena
  SetSize(maxSize);
}
