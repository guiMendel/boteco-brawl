#include "PlatformDrop.h"
#include "PlayerInput.h"

using namespace std;

PlatformDrop::PlatformDrop(WorldObject &associatedObject, shared_ptr<Rigidbody> body)
    : Component(associatedObject), weakBody(body)
{
  // Get player input
  auto playerInput = body->worldObject.RequireComponent<PlayerInput>();

  // Subscribe to fast falling, which is when we should whitelist platforms
  playerInput->OnFastFall.AddListener("whitelist-platform-effectors", [this]()
                                      { WhitelistPlatforms(); });
}

void PlatformDrop::WhitelistPlatforms()
{
  // Get body
  LOCK(weakBody, body);

  auto platformEntryIterator = platformsInRange.begin();
  while (platformEntryIterator != platformsInRange.end())
  {
    IF_LOCK(platformEntryIterator->second, platform)
    {
      platform->Whitelist(body);
      platformEntryIterator++;
    }
    else platformEntryIterator = platformsInRange.erase(platformEntryIterator);
  }
}

void PlatformDrop::OnTriggerCollisionEnter(TriggerCollisionData triggerData)
{
  LOCK(triggerData.weakOther, other);

  // Register this platform as in range
  auto platform = other->worldObject.RequireComponent<PlatformEffector>();
  platformsInRange[platform->id] = platform;
}

void PlatformDrop::OnTriggerCollisionExit(TriggerCollisionData triggerData)
{
  LOCK(triggerData.weakOther, other);

  // Get platform
  auto platform = other->worldObject.RequireComponent<PlatformEffector>();

  // Get body
  LOCK(weakBody, body);

  // Remove possible whitelist
  platform->WhitelistRemove(body);

  // Forget platform
  platformsInRange.erase(platform->id);
}
