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

  // React to back button hover
  auto backButton = GetScene()->RequireUIObject<UIImage>(BACK_BUTTON_IMAGE);
  weak_ptr weakBack{backButton};

  // Make it lighter on mouse over
  auto lightenBackButton = [weakBack](std::shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseEnter)
      return;

    Lock(weakBack)->SetImagePath("./assets/images/character-selection/header/back-button-light.png");
  };
  auto darkenBackButton = [weakBack](std::shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseLeave)
      return;

    Lock(weakBack)->SetImagePath("./assets/images/character-selection/header/back-button.png");
  };

  backButton->OnUIEvent.AddListener("lighten-on-hover", lightenBackButton);
  backButton->OnUIEvent.AddListener("darken-on-hover-end", darkenBackButton);

  // Return to first screen when it's clicked
  auto returnScreen = [this](std::shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseClick)
      return;

    Lock(weakAnimationHandler)->PanContent(0);
  };
  backButton->OnUIEvent.AddListener("return-screen-on-click", returnScreen);
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
