#ifndef __UI_CONTROLLER_SELECTABLE__
#define __UI_CONTROLLER_SELECTABLE__

#include "UIComponent.h"
#include "ControllerDevice.h"

// Maps input from some source (player or AI) to the the creation and dispatch of the corresponding actions
class UIControllerSelectable : public UIComponent
{
public:
  // When a controller selects this component
  EventI<std::shared_ptr<ControllerDevice>> OnControllerSelect;

  // When a controller unselects this component
  EventI<std::shared_ptr<ControllerDevice>> OnControllerUnselect;

  UIControllerSelectable(GameObject &associatedObject);
  virtual ~UIControllerSelectable() {}

  // Register to instances & set up controller input listener
  void Awake() override;
  void OnBeforeDestroy() override;
  void Update(float) override;

  // Get all instances of this component
  static std::vector<std::shared_ptr<UIControllerSelectable>> GetAllInstances();

  // Associate a controller to this component
  void AssociateController(std::shared_ptr<ControllerDevice> controller);

  // Unassociates a controller to this component, if it's associated
  void UnassociateController(std::shared_ptr<ControllerDevice> controller);

  // Controllers associated to this component (by the controller's instance id)
  std::unordered_map<int, std::weak_ptr<ControllerDevice>> associatedControllers;

private:
  // Move an associated controller to the given direction
  void MoveControllerSelection(std::shared_ptr<ControllerDevice> controller, Vector2 direction);

  // Get timer for a controller
  static Timer &GetControllerTimer(std::shared_ptr<ControllerDevice> controller);

  // Provides quick access to all of the existent UIControllerSelectable instances
  static std::vector<std::weak_ptr<UIControllerSelectable>> instances;

  // Maps controller id to a timer
  static std::unordered_map<int, Timer> controllerCooldowns;

  // Callback identifier
  std::string callbackIdentifier;
};

#endif