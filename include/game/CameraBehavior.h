#ifndef __CAMERA_BEHAVIOR__
#define __CAMERA_BEHAVIOR__

#include "Component.h"
#include "Camera.h"
#include "Arena.h"

class CameraBehavior : public Component
{
public:
  CameraBehavior(GameObject &associatedObject, std::shared_ptr<GameObject> charactersParent);
  virtual ~CameraBehavior() {}

  void Awake() override;
  void Start() override;
  void Update(float) override;

  // Resets camera back to initial position & min zoom
  void Reset();

private:
  // Aspect ratio of arena
  float arenaAspectRatio;

  std::weak_ptr<Camera> weakCamera;
  std::weak_ptr<Arena> weakArena;

  // Reference to object which holds each character's objects
  std::weak_ptr<GameObject> weakCharactersParent;
};

#endif