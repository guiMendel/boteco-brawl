#include "GameObject.h"
#include "Component.h"
#include "Game.h"

using namespace std;

Component::Component(GameObject &associatedObject)
    : gameObject(associatedObject), gameState(gameObject.gameState), id(gameState.SupplyId()), inputManager(Game::GetInstance().GetInputManager()) {}

void Component::StartAndRegisterLayer()
{
  if (started)
    return;

  if (GetRenderLayer() != RenderLayer::None)
  {
    Game::GetInstance().GetState().RegisterLayerRenderer(GetShared());
  }

  started = true;

  Start();
}

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
