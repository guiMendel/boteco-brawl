#ifndef __UI_INPUT_MANAGER__
#define __UI_INPUT_MANAGER__

#include "WorldComponent.h"
#include "MouseCursor.h"
#include "UIContainer.h"
#include "SplashAnimation.h"
#include "PlayerManager.h"
#include "BrawlPlayer.h"
#include <queue>

#define BILL_TEXT_IMAGE "SelectionText"
#define SELECTION_IMAGE(playerId) "SelectionBadge" + std::to_string(playerId)
#define HOVER_IMAGE(playerId) "HoverBadge" + std::to_string(playerId)
#define IDLE_ANIMATION_VIEW(playerId) "IdleAnimationView" + std::to_string(playerId)

class MainMenuInput : public WorldComponent
{
public:
  MainMenuInput(GameObject &associatedObject);
  virtual ~MainMenuInput() {}

  void OnBeforeDestroy() override;
  void Start() override;
  void Update(float) override;

  // Creates a new player
  std::shared_ptr<BrawlPlayer> CreatePlayer();

private:
  // Activate hover cursor
  void SetHoverCursor(bool value);

  // Plays click animation if is hovering
  void AnimateClick();

  // Associates a player to a player bill
  void AssociatePlayerBill(std::shared_ptr<BrawlPlayer> player, std::shared_ptr<UIContainer> bill);

  // Gets a bill by it's index
  std::shared_ptr<UIContainer> GetBill(size_t index);

  // Registers input event listeners
  void RegisterListeners();

  // Triggered when start or enter is pressed
  void PlayerStart();

  // Triggered when a controller hits start
  void ControllerStart(std::shared_ptr<ControllerDevice> controller);

  // Sets up selection for a character option
  void SetUpMouseSelection(std::shared_ptr<UIContainer> option);

  // Sets up hover for a character option
  void SetUpMouseHover(std::shared_ptr<UIContainer> option);

  // Set a player to hover an option
  void SetPlayerHover(std::shared_ptr<UIContainer> option, std::shared_ptr<BrawlPlayer> player);

  // Set a player to select an option
  void SetPlayerSelect(std::shared_ptr<UIContainer> option, std::shared_ptr<BrawlPlayer> player);

  // Remove a player's hover
  void RemovePlayerHover(std::shared_ptr<BrawlPlayer> player);

  // Remove a player's selection
  void RemovePlayerSelect(std::shared_ptr<BrawlPlayer> player);

  // Update if start prompt is ready or not to show
  void UpdateStartPrompt();

  // Transition to arena scene
  void StartBattle();

  // Whether ready to start arena battle
  bool arenaStartReady{false};

  // The default cursor
  std::shared_ptr<MouseCursor> defaultCursor;

  // The hover cursor
  std::shared_ptr<MouseCursor> hoverCursor;

  // Whether cursor is in hover mode
  bool hovering{false};

  // Maps player id to respective bill object
  std::unordered_map<int, std::weak_ptr<UIContainer>> playerBills;

  // Maps player id to selected character option
  std::unordered_map<int, std::weak_ptr<UIContainer>> playerSelections;

  // Maps player id to hovered character option
  std::unordered_map<int, std::weak_ptr<UIContainer>> playerHovers;

  std::weak_ptr<SplashAnimation> weakAnimationHandler;
  std::weak_ptr<UIContainer> weakBillContainer;
  std::weak_ptr<PlayerManager> weakPlayerManager;
  std::weak_ptr<Sound> weakSound;

  // Selection badges for each player to take as they connect
  std::queue<std::pair<std::string, std::string>> playerBadges;

  std::string callbackIdentifier;
};

#endif