#ifndef __UI_INPUT_MANAGER__
#define __UI_INPUT_MANAGER__

#include "WorldComponent.h"
#include "UIContainer.h"
#include "SplashAnimation.h"
#include "PlayerManager.h"

#define BILL_TEXT_IMAGE "SelectionText"
#define SELECTION_IMAGE "SelectionBadge"
#define HOVER_IMAGE "HoverBadge"

class MainMenuInput : public WorldComponent
{
public:
  MainMenuInput(GameObject &associatedObject);
  virtual ~MainMenuInput() {}

  void Start() override;

private:
  // Associates a player to a player bill
  void AssociatePlayerBill(std::shared_ptr<Player> player, std::shared_ptr<UIContainer> bill);

  // Gets a bill by it's index
  std::shared_ptr<UIContainer> GetBill(size_t index);

  // Registers input event listeners
  void RegisterListeners();

  // Triggered when start or enter is pressed
  void PlayerStart();

  // Triggered when a controller hits start
  void ControllerStart(std::shared_ptr<ControllerDevice> controller);

  // Sets up selection for a character option
  void SetUpSelection(std::shared_ptr<UIContainer> option);

  // Sets up hover for a character option
  void SetUpHover(std::shared_ptr<UIContainer> option);

  // Maps player id to respective bill object
  std::unordered_map<int, std::weak_ptr<UIContainer>> playerBills;

  // Maps player id to selected character option
  std::unordered_map<int, std::weak_ptr<UIContainer>> playerSelections;

  // Maps player id to hovered character option
  std::unordered_map<int, std::weak_ptr<UIContainer>> playerHovers;

  std::weak_ptr<SplashAnimation> weakAnimationHandler;
  std::weak_ptr<UIContainer> weakBillContainer;
  std::weak_ptr<PlayerManager> weakPlayerManager;
};

#endif