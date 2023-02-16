#include "MainMenuInput.h"
#include "ArenaScene.h"
#include "UIControllerSelectable.h"
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
#define SELECTOR_OFFSET -11

#define SOUND_HOVER_CHAR "character-hover"
#define SOUND_SELECT_CHAR "character-select"
#define SOUND_CONNECT_PLAYER "player-connect"
#define SOUND_START_BATTLE "start-battle"

MainMenuInput::MainMenuInput(GameObject &associatedObject)
    : WorldComponent(associatedObject),
      defaultCursor(make_shared<MouseCursor>("./assets/images/knife-pointer.png", Vector2(0, 12))),
      hoverCursor(make_shared<MouseCursor>("./assets/images/knife-hover.png", Vector2(0, 12))),
      weakAnimationHandler(GetScene()->RequireUIObject<UIContainer>(MAIN_CONTAINER_OBJECT)->RequireComponent<SplashAnimation>()),
      weakBillContainer(GetScene()->RequireUIObject<UIContainer>(BILLS_OBJECT)),
      weakPlayerManager(GetScene()->RequireFindComponent<PlayerManager>()),
      weakSound(GetScene()->RequireFindComponent<Camera>()->worldObject.RequireComponent<Sound>())
{
  // Add player badges
  playerBadges.push({"./assets/images/character-selection/character-options/selector-1.png",
                     "./assets/images/character-selection/character-options/selected-1.png"});
  playerBadges.push({"./assets/images/character-selection/character-options/selector-2.png",
                     "./assets/images/character-selection/character-options/selected-2.png"});
  playerBadges.push({"./assets/images/character-selection/character-options/selector-3.png",
                     "./assets/images/character-selection/character-options/selected-3.png"});
  playerBadges.push({"./assets/images/character-selection/character-options/selector-4.png",
                     "./assets/images/character-selection/character-options/selected-4.png"});

  // Add sounds
  LOCK(weakSound, sound);

  sound->AddAudio(SOUND_HOVER_CHAR, "./assets/sounds/character-hover.mp3");
  sound->AddAudio(SOUND_SELECT_CHAR, "./assets/sounds/character-select.mp3");
  sound->AddAudio(SOUND_CONNECT_PLAYER, "./assets/sounds/player-connect.mp3");
  sound->AddAudio(SOUND_START_BATTLE, "./assets/sounds/battle-start.wav");
}

void MainMenuInput::Start()
{
  // Activate default cursor
  defaultCursor->Activate();

  // Register all listeners
  RegisterListeners();

  // Associate main player to first bill
  AssociatePlayerBill(RequirePointerCast<BrawlPlayer>(Lock(weakPlayerManager)->GetMainPlayer()),
                      GetBill(0));
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

  // Maybe start battle
  if (arenaStartReady)
    StartBattle();
}

void MainMenuInput::RegisterListeners()
{
  callbackIdentifier = "main-menu-input-" + to_string(id);

  // Raises stat if enter was pressed
  auto onKeyPress = [this](int key)
  {
    if (key == SDLK_RETURN)
      PlayerStart();
  };

  inputManager.OnKeyPress.AddListener(callbackIdentifier, onKeyPress);

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
    SetUpMouseSelection(optionContainer);
    SetUpMouseHover(optionContainer);
  }

  // Cursor click animation
  inputManager.OnClickDown.AddListener(callbackIdentifier, [this](Vector2)
                                       { AnimateClick(); });

  // Subscribe to controller hover
  for (auto selectable : UIControllerSelectable::GetAllInstances())
  {
    weak_ptr weakOption{GetScene()->RequireUIObject<UIContainer>(selectable->uiObject.id)};

    auto onControllerHover = [this, weakOption](shared_ptr<ControllerDevice> controller)
    {
      // Ignore controllers with no player
      auto player = RequirePointerCast<BrawlPlayer>(controller->GetPlayer());

      if (player == nullptr)
        return;

      // Add the hover badge
      SetPlayerHover(Lock(weakOption), player);
    };

    auto onControllerUnhover = [this](shared_ptr<ControllerDevice> controller)
    {
      // Ignore controllers with no player
      auto player = RequirePointerCast<BrawlPlayer>(controller->GetPlayer());

      if (player == nullptr)
        return;

      // Remove the hover badge
      RemovePlayerHover(player);
    };

    selectable->OnControllerSelect.AddListener("menu-select", onControllerHover);
    selectable->OnControllerUnselect.AddListener("menu-unselect", onControllerUnhover);
  }

  // Subscribe to controller selection
  auto onButtonPress = [this](SDL_GameControllerButton button, shared_ptr<ControllerDevice> controller)
  {
    // Get start
    if (button == SDL_CONTROLLER_BUTTON_START)
    {
      ControllerStart(controller);
      return;
    }

    // Only accept A
    if (button != SDL_CONTROLLER_BUTTON_A || controller->GetPlayer() == nullptr)
      return;

    // Ignore controller with no player
    auto player = RequirePointerCast<BrawlPlayer>(controller->GetPlayer());

    if (player == nullptr)
      return;

    // If this player has no hover, stop
    if (playerHovers.count(player->PlayerId()) == 0)
      return;

    // Select hovered option
    SetPlayerSelect(Lock(playerHovers[player->PlayerId()]), player);
  };

  inputManager.OnControllerButtonPress.AddListener(callbackIdentifier, onButtonPress);

  // React to battle start prompt hover
  auto startBattlePrompt = GetScene()->RequireUIObject<UIImage>(START_ARENA_IMAGE);

  // Make it lighter on mouse over
  auto startPromptInteractions = [this](shared_ptr<UIEvent> event)
  {
    if (event->GetType() == UIEvent::OnMouseEnter)
      SetHoverCursor(true);

    else if (event->GetType() == UIEvent::OnMouseLeave)
      SetHoverCursor(false);

    else if (event->GetType() == UIEvent::OnMouseClick)
      StartBattle();
  };

  startBattlePrompt->OnUIEvent.AddListener("cursor-on-hover", startPromptInteractions);
}

void MainMenuInput::SetUpMouseSelection(shared_ptr<UIContainer> option)
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

    // Set main player to select this option
    SetPlayerSelect(Lock(weakOption),
                    RequirePointerCast<BrawlPlayer>(Lock(weakPlayerManager)->GetMainPlayer()));
  };

  option->OnUIEvent.AddListener("detect-selection", handleSelection);
}

void MainMenuInput::SetUpMouseHover(shared_ptr<UIContainer> option)
{
  // Get it's data
  auto optionData = option->GetComponent<CharacterUIOption>();

  if (optionData == nullptr)
    return;

  weak_ptr weakOption{option};

  // Handle it's hover
  auto handleHoverEnter = [this, weakOption](shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseEnter)
      return;

    // Set hovering state
    SetHoverCursor(true);

    // Set main player to hover this option
    SetPlayerHover(Lock(weakOption),
                   RequirePointerCast<BrawlPlayer>(Lock(weakPlayerManager)->GetMainPlayer()));
  };

  // Handle it's hover
  auto handleHoverExit = [this, weakOption](shared_ptr<UIEvent> event)
  {
    if (event->GetType() != UIEvent::OnMouseLeave)
      return;

    // Unset hovering
    SetHoverCursor(false);

    // Remove main player hover badge
    RemovePlayerHover(RequirePointerCast<BrawlPlayer>(Lock(weakPlayerManager)->GetMainPlayer()));
  };

  option->OnUIEvent.AddListener("detect-hover-enter", handleHoverEnter);
  option->OnUIEvent.AddListener("detect-hover-exit", handleHoverExit);
}

void MainMenuInput::AssociatePlayerBill(shared_ptr<BrawlPlayer> player, shared_ptr<UIContainer> bill)
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
  // Ignore if already has all players
  if (controller->GetPlayer() == nullptr && playerBadges.empty() == false)
  {
    auto newPlayer = CreatePlayer();

    // Associate this controller to it
    controller->AssociateToPlayer(newPlayer);

    // Associate player to bill
    AssociatePlayerBill(newPlayer, GetBill(playerBills.size()));

    // Start controller hover on first option
    auto firstOptionSelectable = UIControllerSelectable::GetAllInstances().at(0);

    // Associate controller to this selectable
    firstOptionSelectable->AssociateController(controller);
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

void MainMenuInput::SetPlayerHover(shared_ptr<UIContainer> option, shared_ptr<BrawlPlayer> player)
{
  // If there was a previous hover, remove it
  RemovePlayerHover(player);

  // Make new association
  playerHovers[player->PlayerId()] = option;

  Lock(weakSound)->Play(SOUND_HOVER_CHAR);

  // Place hover badge on option
  auto hoverBadge = option->AddChild<UIImage>(HOVER_IMAGE(player->PlayerId()), player->hoverBadgePath);
  hoverBadge->SetPositionAbsolute(true);
  hoverBadge->absolutePosition.x.Set(UIDimension::RealPixels, SELECTOR_OFFSET);
}

void MainMenuInput::SetPlayerSelect(shared_ptr<UIContainer> option, shared_ptr<BrawlPlayer> player)
{
  // If there was a previous selection, remove it
  RemovePlayerSelect(player);

  // Make new association
  playerSelections[player->PlayerId()] = option;

  Lock(weakSound)->Play(SOUND_SELECT_CHAR);

  // Place selected badge on option
  auto selectedBadge = option->AddChild<UIImage>(SELECTION_IMAGE(player->PlayerId()), player->selectionBadgePath);
  selectedBadge->SetPositionAbsolute(true);
  selectedBadge->absolutePosition.x.Set(UIDimension::RealPixels, SELECTOR_OFFSET);

  // Get the data
  auto optionData = option->RequireComponent<CharacterUIOption>();

  // Get the player's bill
  auto bill = Lock(playerBills[player->PlayerId()]);

  // Add a text image
  auto textImage = bill->AddChild<UIImage>(BILL_TEXT_IMAGE, optionData->characterBillTextPath);
  textImage->SetPositionAbsolute(true);
  textImage->offset.Set(UIDimension::RealPixels, Vector2(-6, -37));

  // Add idle animation visualization
  auto billWorldPosition = bill->canvas.CanvasToWorld(bill->GetPosition());
  auto visualizationParent = GetScene()->RequireWorldObject(IDLE_ANIMATIONS_OBJECT);
  auto visualization = visualizationParent->CreateChild(IDLE_ANIMATION_VIEW(player->PlayerId()));
  visualization->SetPosition(billWorldPosition + Vector2{1.7, 2.5});
  visualization->AddComponent<SpriteRenderer>(RenderLayer::UI, 15);
  auto visualizationAnimator = visualization->AddComponent<Animator>();
  optionData->setAnimation(visualizationAnimator);

  // Set the player to this character
  optionData->characterSetter(player);

  // Update start prompt
  UpdateStartPrompt();
}

void MainMenuInput::RemovePlayerHover(shared_ptr<BrawlPlayer> player)
{
  // If there was a previous hover
  if (playerHovers.count(player->PlayerId()) == 0)
    return;

  // Get old option
  LOCK(playerHovers.at(player->PlayerId()), oldOption);

  // Remove old badge
  oldOption->RequireChild(HOVER_IMAGE(player->PlayerId()))->RequestDestroy();

  // Remove option record
  playerHovers.erase(player->PlayerId());
}

void MainMenuInput::RemovePlayerSelect(shared_ptr<BrawlPlayer> player)
{
  // If there was a previous selection
  if (playerSelections.count(player->PlayerId()) == 0)
    return;

  // Get old option
  LOCK(playerSelections.at(player->PlayerId()), oldOption);

  // Remove old badge
  oldOption->RequireChild(SELECTION_IMAGE(player->PlayerId()))->RequestDestroy();

  // Remove option record
  playerSelections.erase(player->PlayerId());

  // Get the player's bill
  auto bill = Lock(playerBills[player->PlayerId()]);

  // Get it's text image
  auto textImage = RequirePointerCast<UIImage>(bill->RequireChild(BILL_TEXT_IMAGE));

  // Remove it
  textImage->RequestDestroy();

  // Destroy animation view
  auto visualizationParent = GetScene()->RequireWorldObject(IDLE_ANIMATIONS_OBJECT);
  auto visualization = visualizationParent->RequireChild(IDLE_ANIMATION_VIEW(player->PlayerId()));
  visualization->RequestDestroy();

  // Remove character from player
  player->characterRecipe = nullptr;

  // Update start prompt
  UpdateStartPrompt();
}

shared_ptr<BrawlPlayer> MainMenuInput::CreatePlayer()
{
  Assert(playerBadges.empty() == false, "Cannot create more players");

  // Get badges for new player
  auto [hoverBadge, selectBadge] = playerBadges.front();
  playerBadges.pop();

  Lock(weakSound)->Play(SOUND_CONNECT_PLAYER);

  // New player
  return Lock(weakPlayerManager)->AddNewPlayer<BrawlPlayer>(hoverBadge, selectBadge);
}

void MainMenuInput::UpdateStartPrompt()
{
  LOCK(weakPlayerManager, playerManager);
  LOCK(weakAnimationHandler, animationHandler);

  // Get players
  auto players = playerManager->GetPlayers();

  // If only one player, not ready
  if (players.size() <= 1)
  {
    arenaStartReady = false;
    animationHandler->targetStartPromptOffset = 100;
    return;
  }

  // Check that all players have a character recipe
  for (auto player : players)
    if (RequirePointerCast<BrawlPlayer>(player)->characterRecipe == nullptr)
    {
      arenaStartReady = false;
      animationHandler->targetStartPromptOffset = 100;
      return;
    }

  arenaStartReady = true;
  animationHandler->targetStartPromptOffset = 0;
}

void MainMenuInput::StartBattle()
{
  if (arenaStartReady == false)
    return;

  // Loads battle scene
  auto loadBattleScene = [this]()
  {
    Game::GetInstance().SetScene(make_shared<ArenaScene>());
  };

  // Transition out of the menu
  Lock(weakAnimationHandler)->TransitionOutAndExecute(loadBattleScene);

  Lock(weakSound)->Play(SOUND_START_BATTLE);
}

void MainMenuInput::OnBeforeDestroy()
{
  inputManager.OnKeyPress.RemoveListener(callbackIdentifier);
  inputManager.OnClickDown.RemoveListener(callbackIdentifier);
  inputManager.OnControllerButtonPress.RemoveListener(callbackIdentifier);
}
