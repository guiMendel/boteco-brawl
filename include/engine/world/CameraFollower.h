#ifndef __CAMERA_FOLLOWER__
#define __CAMERA_FOLLOWER__

#include "WorldComponent.h"
#include "Camera.h"
#include <iostream>

class CameraFollower : public WorldComponent
{
public:
  CameraFollower(GameObject &associatedObject, bool useRawPosition = false)
      : WorldComponent(associatedObject), useRawPosition(useRawPosition) {}

  CameraFollower(GameObject &associatedObject, ComponentParameters &parameters)
      : CameraFollower(associatedObject, parameters["useRawPosition"].asBool) {}

  virtual ~CameraFollower() {}

  void Update([[maybe_unused]] float deltaTime) override
  {
    worldObject.SetPosition(useRawPosition ? Camera::GetMain()->GetTopLeft() : Camera::GetMain()->GetPosition());
  }

  bool useRawPosition;
};

#endif