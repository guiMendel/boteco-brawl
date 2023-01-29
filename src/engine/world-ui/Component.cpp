#include "GameObject.h"
#include "WorldComponent.h"
#include "Game.h"

using namespace std;

Component::Component(GameObject &associatedObject)
    : gameObject(associatedObject),
      id(GetScene()->SupplyId()),
      inputManager(Game::GetInstance().GetInputManager()) {}

Component::~Component() {}

void Component::SetEnabled(bool value) { enabled = value; }

bool Component::IsEnabled() const { return enabled && gameObject.IsEnabled(); }

shared_ptr<GameScene> Component::GetScene() const { return gameObject.GetScene(); }

void Component::RegisterToSceneWithLayer()
{
  RegisterLayer();
  RegisterToScene();
}

void Component::RegisterLayer()
{
  if (GetRenderLayer() != RenderLayer::None)
  {
    Game::GetInstance().GetScene()->RegisterLayerRenderer(gameObject.RequireComponent(this));
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
bool Component::operator!=(const Component &other) const { return !(*this == other); }

Component::operator std::string() const
{
  return "(" + string(typeid(*this).name()) + "::" + to_string(id) + ")>" + (string)gameObject;
}

ostream &operator<<(ostream &stream, const Component &component)
{
  stream << (string)component;
  return stream;
}

bool Component::ShouldRender() { return IsEnabled(); }
