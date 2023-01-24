#include "CameraBehavior.h"
#include "FallOffDeath.h"
#include "BoxCollider.h"
#include "Debug.h"

using namespace std;

const float CameraBehavior::padding{2};

const float CameraBehavior::baseSpeed{4};
const float CameraBehavior::baseSizeSpeed{0.5};

const float CameraBehavior::baseAcceleration{0.1};
const float CameraBehavior::baseSizeAcceleration{0.5};

static const float alignTolerance{DegreesToRadians(35)};

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

Vector2 CameraBehavior::GetFrameDimensions() const
{
  LOCK(weakCamera, camera);

  float cameraHeight = camera->GetSize() * 2;

  return {cameraHeight * screenRatio - padding * 2, cameraHeight - padding * 2};
}

void CameraBehavior::Start()
{
  Reset();
}

void CameraBehavior::Update(float deltaTime)
{
  // Calculate new targets
  UpdateTargets();

  // Move camera position and size towards targets
  MoveTowardsTargets(deltaTime);

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

  // Reset unframed space
  currentlyUnframedSpace = 0;

  // Get frame
  auto frameSize = GetFrameDimensions() / 2;

  for (auto character : charactersParent->GetChildren())
  {
    // Get it's box
    auto characterBox = character->RequireComponent<BoxCollider>()->GetBox();

    // Calculate if this body has unframed space on X axis
    auto characterScreenPosition = (character->GetPosition() - gameObject.GetPosition()).GetAbsolute();

    currentlyUnframedSpace = max(currentlyUnframedSpace,
                                 characterScreenPosition.x + characterBox.width / 2 - frameSize.x);

    currentlyUnframedSpace = max(currentlyUnframedSpace,
                                 characterScreenPosition.y + characterBox.height / 2 - frameSize.y);

    // Keep biggest x distance
    maxDistances.x = max(
        maxDistances.x,
        // abs(character->GetPosition().x - gameObject.GetPosition().x) + characterBox.width / 2);
        abs(character->GetPosition().x - targetPosition.x) + characterBox.width / 2);

    // Keep biggest y distance
    maxDistances.y = max(
        maxDistances.y,
        // abs(character->GetPosition().y - gameObject.GetPosition().y) + characterBox.height / 2);
        abs(character->GetPosition().y - targetPosition.y) + characterBox.height / 2);
  }

  LOCK(weakCamera, camera);

  // Set the size to the largest dimension
  targetSize = max(maxDistances.y, maxDistances.x / screenRatio) + padding;
}

void CameraBehavior::MoveTowardsTargets(float deltaTime)
{
  LOCK(weakCamera, camera);

  // Find this frame's target speeds
  float unframedFactor = max(1.0f, currentlyUnframedSpace * currentlyUnframedSpace);
  float targetSpeed = baseSpeed * unframedFactor;
  float targetSizeSpeed = baseSizeSpeed * unframedFactor;

  // Get speed difference
  float speedDifference = Clamp(targetSpeed - speed, -baseAcceleration, baseAcceleration);
  float sizeSpeedDifference = Clamp(targetSizeSpeed - sizeSpeed, -baseSizeAcceleration, baseSizeAcceleration);

  // Accelerate current speed to target speed
  speed += speedDifference;
  sizeSpeed += sizeSpeedDifference;

  // Get target position direction
  Vector2 targetDisplacement = targetPosition - gameObject.GetPosition();
  float sizeChange = targetSize - camera->GetSize();

  // Apply displacement but cap it to possible speed
  SetPosition(gameObject.GetPosition() + targetDisplacement.CapMagnitude(speed * deltaTime));
  SetSize(camera->GetSize() + Clamp(sizeChange, -sizeSpeed * deltaTime, sizeSpeed * deltaTime));
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
