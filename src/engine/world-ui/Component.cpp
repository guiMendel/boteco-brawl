#include "WorldObject.h"
#include "Component.h"
#include "Game.h"

using namespace std;

Component::Component(WorldObject &associatedObject)
    : worldObject(associatedObject), id(GetState()->SupplyId()), inputManager(Game::GetInstance().GetInputManager()) {}

Component::~Component() {}

shared_ptr<Component> Component::GetShared() const
{
  try
  {
    return worldObject.RequireComponent(this);
  }
  catch (runtime_error &)
  {
    throw runtime_error("Component failed to get own shared pointer: it was not found in it's worldObject list");
  }
}

bool Component::IsEnabled() const { return enabled && worldObject.IsEnabled(); }

shared_ptr<GameState> Component::GetState() const { return worldObject.GetState(); }

void Component::RegisterToStateWithLayer()
{
  RegisterLayer();
  RegisterToState();
}

void Component::RegisterLayer()
{
  if (GetRenderLayer() != RenderLayer::None)
  {
    Game::GetInstance().GetState()->RegisterLayerRenderer(GetShared());
  }
}

void Component::SafeStart()
{
  if (HasCalledStart())
    return;

  started = true;

  Start();
}

bool Component::HasCalledStart() const { return started; }

bool Component::operator==(const Component &other) const { return id == other.id; }

Component::operator std::string() const
{
  return "(" + string(typeid(*this).name()) + "::" + to_string(id) + ")>" + (string)worldObject;
}

ostream &operator<<(ostream &stream, const Component &component)
{
  stream << (string)component;
  return stream;
}
