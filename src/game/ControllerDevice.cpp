#include "ControllerDevice.h"
#include "Player.h"
#include "PlayerManager.h"

using namespace Helper;
using namespace std;

SDL_GameController *OpenController(int index)
{
  auto controller = SDL_GameControllerOpen(index);
  Assert(controller != nullptr, "Failed to open connected controller");
  return controller;
}

ControllerDevice::ControllerDevice(int controllerIndex)
    : controllerStruct(OpenController(controllerIndex), SDL_GameControllerClose),
      instanceId(SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controllerStruct.get())))
{
  cout << "Controller \"" << SDL_GameControllerName(controllerStruct.get()) << "\" added with instance ID " << GetId() << endl;

  // Associate to players searching for a controller, when available
  Game::GetInstance().GetState()->FindObjectOfType<PlayerManager>()->OnPlayerSearchForController.AddListener(
      "add-controller", [this](shared_ptr<Player> player)
      { MaybeAssociateToPlayer(player); });
}

ControllerDevice::~ControllerDevice() { LosePlayer(); }

void ControllerDevice::AssociateToPlayer(std::shared_ptr<Player> player)
{
  cout << "Associating controller " << GetId() << " to player " << player->PlayerId() << endl;

  player->AssociateController(GetShared());
  weakAssociationPlayer = player;
}

std::shared_ptr<ControllerDevice> ControllerDevice::GetShared() const
{
  return Game::GetInstance().GetInputManager().GetController(GetId());
}

void ControllerDevice::LosePlayer()
{
  if (auto player = GetPlayer(); player != nullptr)
    player->LoseController();

  weakAssociationPlayer.reset();
}

std::shared_ptr<Player> ControllerDevice::GetPlayer() const { return weakAssociationPlayer.lock(); }

int ControllerDevice::GetId() const { return instanceId; }

void ControllerDevice::MaybeAssociateToPlayer(std::shared_ptr<Player> player)
{
  // Associate if has no associated player and this player has no associated controller
  if (weakAssociationPlayer.expired() && player->SearchingForController())
    AssociateToPlayer(player);
}

void ControllerDevice::SearchPlayersForAssociation()
{
  for (auto player : Game::GetInstance().GetState()->FindObjectOfType<PlayerManager>()->GetPlayers())
  {
    // When a player is associated, stop
    if (weakAssociationPlayer.expired() == false)
      return;

    MaybeAssociateToPlayer(player);
  }
}
