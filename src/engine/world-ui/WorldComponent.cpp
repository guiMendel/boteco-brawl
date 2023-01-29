#include "WorldObject.h"
#include "WorldComponent.h"
#include "Game.h"

using namespace std;

WorldComponent::WorldComponent(GameObject &associatedObject)
    : Component(associatedObject),
      worldObject(*GetScene()->RequireWorldObject(associatedObject.id)) {}

WorldComponent::~WorldComponent() {}

shared_ptr<WorldComponent> WorldComponent::GetShared() const
{
  auto shared = worldObject.GetComponent(this);

  Assert(shared != nullptr, "WorldComponent failed to get own shared pointer: it was not found in it's worldObject list");

  return RequirePointerCast<WorldComponent>(shared);
}
