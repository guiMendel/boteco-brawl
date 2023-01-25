#include "CameraBehavior.h"
#include "FallOffDeath.h"
#include "BoxCollider.h"
#include "Debug.h"

using namespace std;

const float CameraBehavior::padding{2};

const float CameraBehavior::maxSpeed{5};
const float CameraBehavior::baseSizeSpeed{0.5};

const float CameraBehavior::baseAcceleration{6};
const float CameraBehavior::baseSizeAcceleration{0.5};

const float CameraBehavior::deadZoneRange{0.3};

// Bias applied towards deceleration
const float decelerationBias{1};

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
  LOCK(weakCamera, camera);

  // Calculate new targets
  UpdateTargets();

  // Move camera position and size towards targets
  MoveTowardsTargets(deltaTime);

  // Adjust values to ensure camera is valid
  SetPosition(ConfineToArena(gameObject.GetPosition(), camera->GetSize()));
}

void CameraBehavior::UpdateTargets()
{
  // === NEW POSITION CALCULATION

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
  targetSize = 5;

  // === VALIDATE TARGETS

  // Validate it
  targetPosition = ConfineToArena(targetPosition, targetSize);
}

void CameraBehavior::MoveTowardsTargets(float deltaTime)
{
  LOCK(weakCamera, camera);

  // Find this frame's target speeds
  float unframedFactor = max(1.0f, currentlyUnframedSpace * currentlyUnframedSpace);

  // Distance to target position
  Vector2 targetDisplacement = targetPosition - gameObject.GetPosition();

  // Find how much acceleration will be redirected towards decelerating
  // It's an amount proportional to how unaligned the velocity is to the target position
  float decelerationPortion;

  bool lowSpeed = velocity.SqrMagnitude() <= 0.2 * 0.2;

  // Only decelerate if too close
  if (targetDisplacement.SqrMagnitude() <= deadZoneRange * deadZoneRange)
  {
    // If speed is low enough, simply do nothing
    if (lowSpeed)
    {
      lastAcceleration = Vector2::Zero();
      return;
    }

    decelerationPortion = 1;
  }

  // If going too slow, don't bother decelerating
  else if (lowSpeed)
    decelerationPortion = 0;

  else
    decelerationPortion = InverseLerp(
        0.0f,
        float(M_PI),
        decelerationBias * abs(AngleDistance(velocity.Angle(), targetDisplacement.Angle())));

  // Get acceleration and deceleration
  cout << "decelerationPortion: " << decelerationPortion << endl;
  float scaledAcceleration = baseAcceleration * unframedFactor;
  float acceleration = (1 - decelerationPortion) * scaledAcceleration;
  float deceleration = decelerationPortion * scaledAcceleration;

  // Get velocity portion which is pointing to the target position
  float closingSpeed = Vector2::Dot(velocity, targetDisplacement.Normalized());

  // Find how far from the target we need to start braking, given the current speed
  float brakeDistance = -closingSpeed * closingSpeed / (acceleration * 2);

  // If inside brake distance, revert acceleration to brake
  if (targetDisplacement.SqrMagnitude() <= brakeDistance * brakeDistance)
    acceleration = -acceleration;

  lastAcceleration = (targetDisplacement.Normalized() * acceleration +
                      -velocity.Normalized() * deceleration);

  // Apply both accelerations to current velocity
  velocity = (velocity + lastAcceleration * deltaTime).CapMagnitude(maxSpeed * unframedFactor);

  // Apply velocity to position
  SetPosition(gameObject.GetPosition() + velocity * deltaTime);

  float targetSizeSpeed = baseSizeSpeed * unframedFactor;

  // Get speed difference
  float sizeSpeedDifference = Clamp(targetSizeSpeed - sizeSpeed, -baseSizeAcceleration, baseSizeAcceleration);

  // Accelerate current speed to target speed
  sizeSpeed += sizeSpeedDifference;

  // Get target position direction
  float sizeChange = targetSize - camera->GetSize();

  // Apply displacement but cap it to possible speed
  // SetSize(camera->GetSize() + Clamp(sizeChange, -sizeSpeed * deltaTime, sizeSpeed * deltaTime));
  SetSize(5);
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

Vector2 CameraBehavior::ConfineToArena(Vector2 position, float size)
{
  LOCK(weakArena, arena);

  float horizontalSize = size * screenRatio;

  // Adjust position to fit within arena
  return Vector2(
      Clamp(position.x, -arena->width / 2 + horizontalSize, arena->width / 2 - horizontalSize),
      Clamp(position.y, -arena->height / 2 + size, arena->height / 2 - size));
}

void CameraBehavior::Reset()
{
  LOCK(weakCamera, camera);
  LOCK(weakArena, arena);

  SetPosition({0, 0});

  // Adjust size to be the maximum possible which still fits inside the arena
  SetSize(maxSize);
}

void CameraBehavior::Render()
{
  LOCK(weakCamera, camera);

  // Draw target position
  Debug::DrawCircle(Circle(targetPosition, 0.03), Color::Yellow());

  // Draw actual position
  Debug::DrawCircle(Circle(camera->GetPosition(), 0.05), Color::Pink());

  if (lastAcceleration)
    Debug::DrawArrow(camera->GetPosition(), camera->GetPosition() + lastAcceleration, Color::Pink(), 0.2);
}
