#ifndef __CAMERA_BEHAVIOR__
#define __CAMERA_BEHAVIOR__

#include "WorldComponent.h"
#include "Camera.h"
#include "Arena.h"

class CameraBehavior : public WorldComponent
{
public:
  // How much padding to add between characters and edges of the camera
  static const float padding;

  // Base speed of camera displacement
  static const float maxSpeed;

  // Base speed of camera size change
  static const float baseSizeSpeed;

  // Base acceleration of camera displacement
  static const float baseAcceleration;

  // Base acceleration of camera size change
  static const float baseSizeAcceleration;

  // How far from target camera needs to be to start moving
  static const float deadZoneRange;

  // Minimum camera size
  static const float minSize;

  CameraBehavior(GameObject &associatedObject, std::shared_ptr<WorldObject> charactersParent);
  virtual ~CameraBehavior() {}

  // Get current width and height of frame in units (dimensions of camera discounting padding)
  Vector2 GetFrameDimensions() const;

  void Awake() override;
  void Render() override;
  void Start() override;
  void Update(float) override;

  RenderLayer GetRenderLayer() override { return RenderLayer::Debug; }

private:
  // Adjusts target position and size to be as small as possible while still fitting all characters
  void UpdateTargets();

  // Move camera position towards target
  void ApplyTargetPosition(float deltaTime);

  // Resize camera towards target
  void ApplyTargetSize(float deltaTime);

  // Get a multiplier for target approximation based on the currently unframed space
  float GetUnframedFactor() const;

  // Moves camera to the given position
  void SetPosition(Vector2 position);

  // Changes camera's size to the given size
  void SetSize(float newSize);

  // Adjusts given position to ensure that it fits within the arena
  Vector2 ConfineToArena(Vector2 position, float size);

  // Resets camera to center with maximum size
  void Reset();

  // Position to move camera towards
  Vector2 targetPosition;

  // Size to change camera to
  float targetSize;

  // Current velocity
  Vector2 velocity;

  // Last acceleration applied (for debug only)
  Vector2 lastAcceleration;

  // Size speed
  float sizeSpeed{0};

  // Max distance that any character's body is beyond the current frame of the camera
  float currentlyUnframedSpace{0};

  // Aspect ratio of arena
  float arenaAspectRatio;

  // Max size of camera, dictated by arena size
  float maxSize;

  std::weak_ptr<Camera> weakCamera;
  std::weak_ptr<Arena> weakArena;

  // Reference to object which holds each character's objects
  std::weak_ptr<WorldObject> weakCharactersParent;
};

#endif