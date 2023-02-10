#include "MainMenuInput.h"
#include "PlayerManager.h"
#include "MenuScene.h"
#include "InputManager.h"

using namespace std;

MainMenuInput::MainMenuInput(GameObject &associatedObject)
    : WorldComponent(associatedObject),
      weakAnimationHandler(GetScene()->RequireUIObject<UIContainer>(MAIN_CONTAINER_OBJECT)->RequireComponent<SplashAnimation>()),
      weakBillContainer(GetScene()->RequireUIObject<UIContainer>(BILLS_OBJECT)),
      weakPlayerManager(GetScene()->RequireFindComponent<PlayerManager>()) {}

void MainMenuInput::Start()
{
  RegisterListeners();

  // Associate main player to first bill
  AssociatePlayerBill(Lock(weakPlayerManager)->GetMainPlayer(), GetBill(0));
}

void MainMenuInput::PlayerStart()
{
  // Get animation handler
  LOCK(weakAnimationHandler, animationHandler);

  // Pan to selection screen
  animationHandler->PanContent(1);

  // Reset any active animations
  animationHandler->ResetInitialAnimation();
}

void MainMenuInput::RegisterListeners()
{
  // Raises stat if enter was pressed
  auto onKeyPress = [this](int key)
  {
    if (key == SDLK_RETURN)
      PlayerStart();
  };

  inputManager.OnKeyPress.AddListener("ui-start", onKeyPress);

  // Raises stat if start was pressed
  auto onButtonPress = [this](SDL_GameControllerButton button, shared_ptr<ControllerDevice> controller)
  {
    cout << "Pressed button " << button << endl;

    if (button == SDL_CONTROLLER_BUTTON_START)
      ControllerStart(controller);
  };

  inputManager.OnControllerButtonPress.AddListener("ui-start", onButtonPress);
}

void MainMenuInput::AssociatePlayerBill(shared_ptr<Player> player, shared_ptr<UIContainer> bill)
{
  // Remove bill offset
  Lock(weakAnimationHandler)->raisingBills.push_back(bill->RequireChild(BILL_IMAGE));

  // Add entry
  playerBills[player->PlayerId()] = bill;
}

shared_ptr<UIContainer> MainMenuInput::GetBill(size_t index)
{
  return RequirePointerCast<UIContainer>(Lock(weakBillContainer)->GetChildren().at(index));
}

void MainMenuInput::ControllerStart(shared_ptr<ControllerDevice> controller)
{
  PlayerStart();

  // If controller has no player, create one for it
  if (controller->GetPlayer() == nullptr)
  {
    // New player
    auto newPlayer = Lock(weakPlayerManager)->AddNewPlayer();

    // Associate this controller to it
    controller->AssociateToPlayer(newPlayer);

    // Associate player to bill
    AssociatePlayerBill(newPlayer, GetBill(playerBills.size()));
  }
}
