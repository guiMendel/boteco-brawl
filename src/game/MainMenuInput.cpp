#include "MainMenuInput.h"
#include "CharacterUIOption.h"
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
  auto lightenBackButton = [weakBack](shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseEnter)
      return;

    Lock(weakBack)->SetImagePath("./assets/images/character-selection/header/back-button-light.png");
  };
  auto darkenBackButton = [weakBack](shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseLeave)
      return;

    Lock(weakBack)->SetImagePath("./assets/images/character-selection/header/back-button.png");
  };

  backButton->OnUIEvent.AddListener("lighten-on-hover", lightenBackButton);
  backButton->OnUIEvent.AddListener("darken-on-hover-end", darkenBackButton);

  // Return to first screen when it's clicked
  auto returnScreen = [this](shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseClick)
      return;

    Lock(weakAnimationHandler)->PanContent(0);
  };

  backButton->OnUIEvent.AddListener("return-screen-on-click", returnScreen);

  // For each character option
  for (auto option : GetScene()->RequireUIObject<UIContainer>(OPTIONS_OBJECT)->GetChildren())
  {
    auto optionContainer = RequirePointerCast<UIContainer>(option);
    SetUpSelection(optionContainer);
    SetUpHover(optionContainer);
  }
}

void MainMenuInput::SetUpSelection(shared_ptr<UIContainer> option)
{
  // Get it's data
  auto optionData = option->GetComponent<CharacterUIOption>();

  if (optionData == nullptr)
    return;

  weak_ptr weakData{optionData};
  weak_ptr weakOption{option};

  // Handle it's selection
  auto handleSelection = [this, weakData, weakOption](shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseClick)
      return;

    // Get this player
    auto player = Lock(weakPlayerManager)->GetMainPlayer();

    // If there was a previous selection
    if (playerSelections.count(player->PlayerId()) > 0)
    {
      LOCK(playerSelections.at(player->PlayerId()), oldOption);

      // Remove old badge
      oldOption->RequireChild(SELECTION_IMAGE)->RequestDestroy();
    }

    // Make new association
    LOCK(weakOption, option);
    playerSelections[player->PlayerId()] = option;

    // Place selected badge on option
    auto selectedBadge = option->AddChild<UIImage>(SELECTION_IMAGE, "./assets/images/character-selection/character-options/selected-1.png");
    selectedBadge->SetPositionAbsolute(true);
    selectedBadge->absolutePosition.x.Set(UIDimension::RealPixels, -20);

    LOCK(weakData, optionData);

    // Get the player's bill
    auto bill = Lock(playerBills[player->PlayerId()]);

    // See if it already has a text image
    auto textImage = dynamic_pointer_cast<UIImage>(bill->GetChild(BILL_TEXT_IMAGE));

    // If not, add it
    if (textImage == nullptr)
    {
      textImage = bill->AddChild<UIImage>(BILL_TEXT_IMAGE, optionData->characterBillTextPath);
      textImage->SetPositionAbsolute(true);
      textImage->offset.Set(UIDimension::RealPixels, Vector2(-6, -37));
    }

    else
      textImage->SetImagePath(optionData->characterBillTextPath);
  };

  option->OnUIEvent.AddListener("detect-selection", handleSelection);
}

void MainMenuInput::SetUpHover(shared_ptr<UIContainer> option)
{
  // Get it's data
  auto optionData = option->GetComponent<CharacterUIOption>();

  if (optionData == nullptr)
    return;

  weak_ptr weakOption{option};

  // Handle it's selection
  auto handleHoverEnter = [this, weakOption](shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseEnter)
      return;

    // Get this player
    auto player = Lock(weakPlayerManager)->GetMainPlayer();

    // If there was a previous hover
    if (playerHovers.count(player->PlayerId()) > 0)
    {
      LOCK(playerHovers.at(player->PlayerId()), oldOption);

      // Remove old badge
      oldOption->RequireChild(HOVER_IMAGE)->RequestDestroy();
    }

    // Make new association
    LOCK(weakOption, option);
    playerHovers[player->PlayerId()] = option;

    // Place hover badge on option
    auto hoverBadge = option->AddChild<UIImage>(HOVER_IMAGE, "./assets/images/character-selection/character-options/selector-1.png");
    hoverBadge->SetPositionAbsolute(true);
    hoverBadge->absolutePosition.x.Set(UIDimension::RealPixels, -20);
  };

  // Handle it's selection
  auto handleHoverExit = [this, weakOption](shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseLeave)
      return;

    // Get this player
    auto player = Lock(weakPlayerManager)->GetMainPlayer();

    // If there was a previous hover
    if (playerHovers.count(player->PlayerId()) > 0)
    {
      LOCK(playerHovers.at(player->PlayerId()), oldOption);

      // Remove old badge
      oldOption->RequireChild(HOVER_IMAGE)->RequestDestroy();

      playerHovers.erase(player->PlayerId());
    }
  };

  option->OnUIEvent.AddListener("detect-hover-enter", handleHoverEnter);
  option->OnUIEvent.AddListener("detect-hover-exit", handleHoverExit);
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
