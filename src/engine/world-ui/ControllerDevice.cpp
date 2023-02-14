#include "ControllerDevice.h"
#include "Player.h"
#include "PlayerManager.h"

using namespace Helper;
using namespace std;

SDL_GameController *OpenController(int index)
{
  auto controller = SDL_GameControllerOpen(index);

  // Throw if failed to get a controller
  if (controller == nullptr)
    throw invalid_controller_error("Failed to open connected controller: " + string(SDL_GetError()));

  return controller;
}

ControllerDevice::ControllerDevice(int controllerIndex)
    : controllerStruct(OpenController(controllerIndex), SDL_GameControllerClose),
      instanceId(SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controllerStruct.get())))
{
  MESSAGE << "Controller \"" << SDL_GameControllerName(controllerStruct.get()) << "\" added with instance ID " << GetId() << endl;

  // Associate to players searching for a controller, when available
  auto playerManager = Game::GetInstance().GetScene()->FindComponent<PlayerManager>();

  if (playerManager == nullptr)
    MESSAGE << "WARNING: Controller " << instanceId << " found no PlayerManager instance." << endl;

  else
    RegisterPlayerManager(playerManager);
}

// TODO: dont destroy controller on scene transition!!!
ControllerDevice::~ControllerDevice()
{
  OnBeforeDestroy.Invoke();

  LosePlayer();
}

void ControllerDevice::AssociateToPlayer(std::shared_ptr<Player> player)
{
  // Erase any previous associations
  if (GetPlayer() != nullptr)
    GetPlayer()->LoseController();

  LosePlayer();

  MESSAGE << "Associating controller " << GetId() << " to player " << player->PlayerId() << endl;

  player->AssociateController(GetShared());
  weakAssociationPlayer = player;

  // Set controller color to player color
  auto color = player->GetColor();
  SDL_GameControllerSetLED(controllerStruct.get(), color.red, color.green, color.blue);
}

std::shared_ptr<ControllerDevice> ControllerDevice::GetShared() const
{
  return Game::GetInstance().GetInputManager().GetController(GetId());
}

void ControllerDevice::LosePlayer()
{
  if (auto oldPlayer{GetPlayer()}; oldPlayer != nullptr)
  {
    weakAssociationPlayer.reset();

    oldPlayer->LoseController();
  }

  weakAssociationPlayer.reset();
}

std::shared_ptr<Player> ControllerDevice::GetPlayer() const { return weakAssociationPlayer.lock(); }

int ControllerDevice::GetId() const { return instanceId; }

void ControllerDevice::MaybeAssociateToPlayer(std::shared_ptr<Player> player)
{
  // Associate if has no associated player and this player has no associated controller
  if (SearchingForPlayer() && player->SearchingForController())
    AssociateToPlayer(player);
}

void ControllerDevice::SearchPlayersForAssociation()
{
  auto playerManager = Game::GetInstance().GetScene()->FindComponent<PlayerManager>();

  if (playerManager == nullptr)
    return;

  for (auto player : playerManager->GetPlayers())
  {
    // When a player is associated, stop
    if (weakAssociationPlayer.expired() == false)
      return;

    MaybeAssociateToPlayer(player);
  }
}

void ControllerDevice::RegisterPlayerManager(std::shared_ptr<PlayerManager> playerManager)
{
  playerManager->OnPlayerSearchForController.AddListener("add-controller", [this](shared_ptr<Player> player)
                                                         { MaybeAssociateToPlayer(player); });
}

bool ControllerDevice::SearchingForPlayer() const { return weakAssociationPlayer.expired(); }
