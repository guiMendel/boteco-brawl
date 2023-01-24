#ifndef __CAMERA_BEHAVIOR__
#define __CAMERA_BEHAVIOR__

#include "Component.h"
#include "Camera.h"
#include "Arena.h"

class CameraBehavior : public Component
{
public:
  // How much padding to add between characters and edges of the camera
  static const float padding;

  CameraBehavior(GameObject &associatedObject, std::shared_ptr<GameObject> charactersParent);
  virtual ~CameraBehavior() {}

  void Awake() override;
  void Start() override;
  void Update(float) override;

  // Moves camera to the given position and zooms out as much as possible
  void MoveTo(Vector2 position);

  // Adjusts camera size to be as small as possible while still fitting all characters
  // May reposition it to ensure it also fits within the arena
  void FitCharacters();

private:
  // Resets camera to center with maximum size
  void Reset();

  // Aspect ratio of arena
  float arenaAspectRatio;

  // Max size of camera, dictated by arena size
  float maxSize;

  std::weak_ptr<Camera> weakCamera;
  std::weak_ptr<Arena> weakArena;

  // Reference to object which holds each character's objects
  std::weak_ptr<GameObject> weakCharactersParent;
};

#endif