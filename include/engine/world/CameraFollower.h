#ifndef __CAMERA_FOLLOWER__
#define __CAMERA_FOLLOWER__

#include "Component.h"
#include "Camera.h"
#include <iostream>

class CameraFollower : public Component
{
public:
  CameraFollower(WorldObject &associatedObject, bool useRawPosition = false)
      : Component(associatedObject), useRawPosition(useRawPosition) {}

  CameraFollower(WorldObject &associatedObject, ComponentParameters &parameters)
      : CameraFollower(associatedObject, parameters["useRawPosition"].asBool) {}

  virtual ~CameraFollower() {}

  void Update([[maybe_unused]] float deltaTime) override
  {
    worldObject.SetPosition(useRawPosition ? Camera::GetMain()->GetTopLeft() : Camera::GetMain()->GetPosition());
  }

  bool useRawPosition;
};

#endif