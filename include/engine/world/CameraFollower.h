#ifndef __CAMERA_FOLLOWER__
#define __CAMERA_FOLLOWER__

#include "Component.h"
#include "Camera.h"
#include <iostream>

class CameraFollower : public Component
{
public:
  CameraFollower(GameObject &associatedObject, bool useRawPosition = false)
      : Component(associatedObject), useRawPosition(useRawPosition) {}

  CameraFollower(GameObject &associatedObject, ComponentParameters &parameters)
      : CameraFollower(associatedObject, parameters["useRawPosition"].asBool) {}

  virtual ~CameraFollower() {}

  void Update([[maybe_unused]] float deltaTime) override
  {
    gameObject.SetPosition(useRawPosition ? Camera::GetMain()->GetTopLeft() : Camera::GetMain()->GetPosition());
  }

  bool useRawPosition;
};

#endif