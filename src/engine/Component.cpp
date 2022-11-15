#include "GameObject.h"
#include "Component.h"
#include "Game.h"

using namespace std;

Component::Component(GameObject &associatedObject)
    : gameObject(associatedObject), gameState(gameObject.gameState), inputManager(InputManager::GetInstance()) {}

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
  return gameObject.GetComponent(this);
}

bool Component::IsEnabled() const { return enabled && gameObject.IsEnabled(); }
