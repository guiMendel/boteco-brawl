#include "Player.h"
#include "PlayerManager.h"
#include "ControllerDevice.h"
#include <typeinfo>

using namespace std;

Player::Player(GameObject &associatedObject, PlayerManager &manager, Color color)
    : WorldComponent(associatedObject), color(color), playerManager(manager) {}

// Is searching when need a controller but has no controller
bool Player::SearchingForController() const { return usingController && weakController.expired(); }

void Player::LoseController()
{
  if (auto currentController = GetController(); currentController != nullptr)
    currentController->LosePlayer();

  // If using a controller, start searching for a new one
  if (usingController)
    SearchForController();

  // Otherwise just forget the current controller
  else
    weakController.reset();
}

void Player::UseController()
{
  usingController = true;

  if (weakController.expired())
    SearchForController();
}

void Player::SearchForController()
{
  cout << "Player " << PlayerId() << " searching for controller" << endl;

  usingController = true;
  weakController.reset();
  playerManager.OnPlayerSearchForController.Invoke(RequirePointerCast<Player>(GetShared()));
}

bool Player::IsMain() const { return playerManager.GetMainPlayer()->id == id; }

shared_ptr<ControllerDevice> Player::GetController() const { return weakController.lock(); }

void Player::AssociateController(shared_ptr<ControllerDevice> newDevice)
{
  if (auto currentController = GetController(); currentController != nullptr)
    currentController->LosePlayer();

  Assert(newDevice != nullptr, "Passing null controller to player");

  weakController = newDevice;
  usingController = true;
}

int Player::PlayerId() const { return worldObject.id; }

Color Player::GetColor() const { return color; }
