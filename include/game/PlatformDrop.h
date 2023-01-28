#ifndef __PLATFORM_DROP__
#define __PLATFORM_DROP__

#include "WorldObject.h"
#include "Component.h"
#include "PlayerInput.h"
#include "PlatformEffector.h"
#include <unordered_map>

class Rigidbody;

class PlatformDrop : public Component
{
public:
  PlatformDrop(WorldObject &associatedObject, std::shared_ptr<Rigidbody> body);
  virtual ~PlatformDrop() {}

  // Register platform as a platform which is in range
  void OnTriggerCollisionEnter(TriggerCollisionData triggerData) override;

  // Remove a possible whitelist and forget platform
  void OnTriggerCollisionExit(TriggerCollisionData triggerData) override;

private:
  // Whitelists all platform which are in range
  void WhitelistPlatforms();

  // Body reference
  std::weak_ptr<Rigidbody> weakBody;

  // Which platforms are currently whitelisting this body
  std::unordered_map<int, std::weak_ptr<PlatformEffector>> platformsInRange;
};

#endif