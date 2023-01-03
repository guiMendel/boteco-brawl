#include "GameObject.h"
#include "Component.h"
#include "Game.h"

using namespace std;

Component::Component(GameObject &associatedObject)
    : gameObject(associatedObject), id(GetState()->SupplyId()), inputManager(Game::GetInstance().GetInputManager()) {}

Component::~Component() {}

shared_ptr<Component> Component::GetShared() const
{
  try
  {
    return gameObject.RequireComponent(this);
  }
  catch (runtime_error &)
  {
    throw runtime_error("Component failed to get own shared pointer: it was not found in it's gameObject list");
  }
}

bool Component::IsEnabled() const { return enabled && gameObject.IsEnabled(); }

shared_ptr<GameState> Component::GetState() const { return gameObject.GetState(); }

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
  if (started)
    return;
  started = true;
  Start();
}

bool Component::operator==(const Component &other) const { return id == other.id; }

Component::operator std::string() const
{
  return "(" + string(typeid(*this).name()) + "::" + to_string(id) + ")>" + (string)gameObject;
}

ostream &operator<<(ostream &stream, const Component &component)
{
  stream << (string)component;
  return stream;
}
