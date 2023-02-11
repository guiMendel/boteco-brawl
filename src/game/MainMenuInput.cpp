#include "MainMenuInput.h"
#include "SpriteRenderer.h"
#include "CharacterUIOption.h"
#include "PlayerManager.h"
#include "MenuScene.h"
#include "InputManager.h"

using namespace std;

// Time that knife cursor remains down after a click
static const float clickAnimationDelay{0.1};

// Speed of cut decay
static const int cutDecaySpeed{200};

// Name of cursor animation delay timer
#define RESET_HOVER_TIMER "reset-hover"

MainMenuInput::MainMenuInput(GameObject &associatedObject)
    : WorldComponent(associatedObject),
      defaultCursor(make_shared<MouseCursor>("./assets/images/knife-pointer.png", Vector2(0, 12))),
      hoverCursor(make_shared<MouseCursor>("./assets/images/knife-hover.png", Vector2(0, 12))),
      weakAnimationHandler(GetScene()->RequireUIObject<UIContainer>(MAIN_CONTAINER_OBJECT)->RequireComponent<SplashAnimation>()),
      weakBillContainer(GetScene()->RequireUIObject<UIContainer>(BILLS_OBJECT)),
      weakPlayerManager(GetScene()->RequireFindComponent<PlayerManager>())
{
}

void MainMenuInput::Start()
{
  // Activate default cursor
  defaultCursor->Activate();

  RegisterListeners();

  // Associate main player to first bill
  AssociatePlayerBill(Lock(weakPlayerManager)->GetMainPlayer(), GetBill(0));
}

void MainMenuInput::Update(float deltaTime)
{
  // Check if a hover click animation is up
  if (hovering && defaultCursor->IsActive() && gameObject.timer.Get(RESET_HOVER_TIMER) >= 0)
  {
    gameObject.timer.Stop(RESET_HOVER_TIMER);

    hoverCursor->Activate();
  }

  // For each cut sprite
  for (auto cutObject : worldObject.GetChildren())
  {
    auto cutSprite = cutObject->RequireComponent<SpriteRenderer>();

    // Get new alpha
    int newAlpha = cutSprite->GetColors().first.alpha - cutDecaySpeed * deltaTime;

    // If it's over, remove child
    if (newAlpha <= 0)
      cutObject->RequestDestroy();

    else
      cutSprite->SetColor(Color(255, 255, 255, newAlpha));
  }
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
  auto lightenBackButton = [this, weakBack](shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseEnter)
      return;

    SetHoverCursor(true);

    Lock(weakBack)->SetImagePath("./assets/images/character-selection/header/back-button-light.png");
  };
  auto darkenBackButton = [this, weakBack](shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseLeave)
      return;

    SetHoverCursor(false);

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

  // Cursor click animation
  inputManager.OnClickDown.AddListener("cursor-animation", [this](Vector2)
                                       { AnimateClick(); });
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

    // Set hovering state
    SetHoverCursor(true);

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

    SDL_Surface *surface = IMG_Load("./assets/images/knife-hover.png");
    SDL_Cursor *cursor = SDL_CreateColorCursor(surface, 0, 12);
    SDL_SetCursor(cursor);
  };

  // Handle it's selection
  auto handleHoverExit = [this, weakOption](shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseLeave)
      return;

    // Unset hovering
    SetHoverCursor(false);

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

    SDL_Surface *surface = IMG_Load("./assets/images/knife-pointer.png");
    SDL_Cursor *cursor = SDL_CreateColorCursor(surface, 0, 12);
    SDL_SetCursor(cursor);
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

void MainMenuInput::SetHoverCursor(bool value)
{
  hovering = value;

  if (hovering)
    hoverCursor->Activate();

  else
    defaultCursor->Activate();
}

void MainMenuInput::AnimateClick()
{
  if (hovering == false || defaultCursor->IsActive())
    return;

  // Activate default cursor
  defaultCursor->Activate();

  // Set timer to revert back to hover cursor
  gameObject.timer.Reset(RESET_HOVER_TIMER, -clickAnimationDelay);

  // Create a cut line sprite
  auto cutLine = worldObject.CreateChild("KnifeCursorCut");

  // Get cut position
  auto cutPosition = GetScene()->inputManager.GetMouseWorldCoordinates() + Vector2(0.25, 0.05);
  cutLine->SetPosition(cutPosition);

  // Add the sprite
  auto cutSprite = cutLine->AddComponent<SpriteRenderer>("./assets/images/knife-cut.png", RenderLayer::UI, 15);
  cutSprite->OverrideWidthPixels(32);
}
