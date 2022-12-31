#ifndef __PLATFORM_EFFECTOR__
#define __PLATFORM_EFFECTOR__

#include "GameObject.h"
#include "Component.h"
#include <utility>
#include <memory>
#include <unordered_set>

class Rigidbody;

// Allows for one-side collision
class PlatformEffector : public Component
{
public:
  PlatformEffector(GameObject &associatedObject, std::pair<float, float> passThroughArc = defaultPassThroughArc);

  virtual ~PlatformEffector() {}

  void Update(float) override;

  // Whether the given body should or not be allowed to not collide with this platform at this frame
  bool AllowThrough(std::shared_ptr<Rigidbody> body);

  // As long as the given body is whitelisted, it will never collide with this platform
  void Whitelist(std::shared_ptr<Rigidbody> body);

  // Remove body from whitelist
  void WhitelistRemove(std::shared_ptr<Rigidbody> body);

private:
  // Whether the given body's velocity is within the allowed arc
  bool IsBodyInArc(std::shared_ptr<Rigidbody> body);

  // Registers a body as being allowed through in this frame
  void RegisterAllowedBody(std::shared_ptr<Rigidbody> body);

  // (In radians) Arc through which, if a body's velocity is within, it will be allowed to pass through
  std::pair<float, float> passThroughArc;

  // Currently whitelisted bodies' ids
  std::unordered_set<int> whitelistedIds;

  // Bodies's ids that were allowed through at this frame
  std::unordered_set<int> allowedIds;

  // Bodies's ids that were allowed through last frame
  std::unordered_set<int> lastAllowedIds;

  // Default value for pass through arc
  static const std::pair<float, float> defaultPassThroughArc;
};

#endif