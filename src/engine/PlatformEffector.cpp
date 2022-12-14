#include "PlatformEffector.h"

using namespace std;

const pair<float, float> PlatformEffector::defaultPassThroughArc{-0.1f, -M_PI + 0.1f};

PlatformEffector::PlatformEffector(GameObject &associatedObject, pair<float, float> passThroughArc)
    : Component(associatedObject), passThroughArc(passThroughArc) {}

void PlatformEffector::PhysicsUpdate(float)
{
  // Update last frame's variable
  lastAllowedIds = allowedIds;
  allowedIds.clear();
}

// As long as the given body is whitelisted, it will never collide with this platform
void PlatformEffector::Whitelist(shared_ptr<Rigidbody> body)
{
  whitelistedIds.insert(body->id);
}

// Remove body from whitelist
void PlatformEffector::WhitelistRemove(shared_ptr<Rigidbody> body)
{
  whitelistedIds.erase(body->id);
}

// Whether the given body should or not be allowed to not collide with this platform at this frame
bool PlatformEffector::AllowThrough(shared_ptr<Rigidbody> body)
{
  // if (whitelistedIds.count(body->id) > 0)
  //   cout << "Allowed by whitelist" << endl;

  // else if (allowedIds.count(body->id) > 0)
  //   cout << "Already allowed this frame" << endl;

  // else if (lastAllowedIds.count(body->id) > 0)
  //   cout << "Allowed by sequence" << endl;

  // else if (IsBodyInArc(body))
  //   cout << "Allowed by velocity arc" << endl;

  // else
  //   cout << "NOT allowed" << endl;

  // Allow if body is whitelisted, or was allowed this or the very last frame, or is within the arc
  if (whitelistedIds.count(body->id) > 0 ||
      allowedIds.count(body->id) > 0 ||
      lastAllowedIds.count(body->id) > 0 ||
      IsBodyInArc(body))
  {
    RegisterAllowedBody(body);
    return true;
  }

  return false;
}

bool PlatformEffector::IsBodyInArc(shared_ptr<Rigidbody> body)
{
  // Ignore irrelevant speeds
  if (body->velocity.SqrMagnitude() < 0.001f)
    return false;

  float velocityAngle = body->velocity.Angle();

  return (passThroughArc.first <= velocityAngle && velocityAngle <= passThroughArc.second) ||
         (passThroughArc.second <= velocityAngle && velocityAngle <= passThroughArc.first);
}

void PlatformEffector::RegisterAllowedBody(shared_ptr<Rigidbody> body)
{
  allowedIds.insert(body->id);
}
