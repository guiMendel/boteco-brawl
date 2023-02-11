#include "UIControllerSelectable.h"

using namespace std;

// Max difference between target direction and component direction that is still valid, in radians
static const float maxDistanceDifference{DegreesToRadians(45)};

// Controller selection move cooldown
static const float selectionMoveCooldown{0.8};

// Cooldown timer
#define COOLDOWN_TIMER "selection-cooldown"

vector<weak_ptr<UIControllerSelectable>> UIControllerSelectable::instances{};
unordered_map<int, Timer> UIControllerSelectable::controllerCooldowns{};

UIControllerSelectable::UIControllerSelectable(GameObject &associatedObject)
    : UIComponent(associatedObject) {}

void UIControllerSelectable::Awake()
{
  // Register to instances
  instances.push_back(RequirePointerCast<UIControllerSelectable>(GetShared()));

  // cout << "adding new instance" << endl;

  // for (auto instance : instances)
  //   cout << "instance " << Lock(instance)->uiObject << endl;

  // Reset a controller timer
  auto resetTimer = [](Timer &timer)
  {
    timer.Reset(COOLDOWN_TIMER, 0, false);
  };

  // Handles a controller analog input
  auto onAnalog = [this, resetTimer](Vector2 direction, shared_ptr<ControllerDevice> targetController)
  {
    // Check if the target controller is currently associated to this component
    if (associatedControllers.count(targetController->GetId()) == 0)
      return;

    // Get this player's timer
    auto &timer = GetControllerTimer(targetController);

    // If direction is null
    if (!direction)
    {
      // Reset timer
      resetTimer(timer);

      // Nothing more
      return;
    }

    // Check that cooldown is up
    if (timer.Get(COOLDOWN_TIMER) < 0)
      return;

    // Move this controller's selection to the given direction
    MoveControllerSelection(targetController, direction.Normalized());

    // Reset cooldown
    timer.Reset(COOLDOWN_TIMER, -selectionMoveCooldown);
  };

  // Handles a controller button
  auto onButton = [this, onAnalog](SDL_GameControllerButton button, std::shared_ptr<ControllerDevice> targetController)
  {
    if (button == SDL_CONTROLLER_BUTTON_DPAD_UP)
      onAnalog(Vector2::Up(), targetController);
    else if (button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
      onAnalog(Vector2::Right(), targetController);
    else if (button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
      onAnalog(Vector2::Down(), targetController);
    else if (button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
      onAnalog(Vector2::Left(), targetController);
  };

  auto onButtonUp = [resetTimer](SDL_GameControllerButton button, std::shared_ptr<ControllerDevice> targetController)
  {
    // Ignore if not dpad
    if (button != SDL_CONTROLLER_BUTTON_DPAD_UP &&
        button != SDL_CONTROLLER_BUTTON_DPAD_RIGHT &&
        button != SDL_CONTROLLER_BUTTON_DPAD_DOWN &&
        button != SDL_CONTROLLER_BUTTON_DPAD_LEFT)
      return;

    // Reset this controller's timer
    resetTimer(GetControllerTimer(targetController));
  };

  // Set up listeners
  GetScene()->inputManager.OnControllerLeftAnalog.AddListener("controller-selectable-" + to_string(id), onAnalog);
  GetScene()->inputManager.OnControllerButtonPress.AddListener("controller-selectable-" + to_string(id), onButton);
  GetScene()->inputManager.OnControllerButtonRelease.AddListener("controller-selectable-reset-timer-" + to_string(id), onButtonUp);
}

void UIControllerSelectable::MoveControllerSelection(shared_ptr<ControllerDevice> controller, Vector2 targetDirection)
{
  // The best next component candidate found
  shared_ptr<UIControllerSelectable> bestCandidate;

  // Angle distance of best candidate so far
  float bestAngleDistance{maxDistanceDifference};

  // For each other instance
  for (auto instance : GetAllInstances())
  {
    if (instance->id == id)
      continue;

    // Get the instance direction from here
    Vector2 instanceDirection = (instance->uiObject.GetPosition() - uiObject.GetPosition()).Normalized();

    // Get it's angle distance
    float angleDistance = abs(AngleDistance(instanceDirection.Angle(), targetDirection.Angle()));

    if (angleDistance < bestAngleDistance)
    {
      bestAngleDistance = angleDistance;
      bestCandidate = instance;
    }
  }

  // If there is no valid candidate, stop
  if (bestCandidate == nullptr)
    return;

  // Associate controller to new candidate
  bestCandidate->AssociateController(controller);
}

vector<shared_ptr<UIControllerSelectable>> UIControllerSelectable::GetAllInstances()
{
  // cout << "getting instances" << endl;

  // for (auto instance : instances)
  //   cout << "instance " << Lock(instance)->uiObject << endl;

  return ParseWeakIntoShared(instances);
}

void UIControllerSelectable::AssociateController(std::shared_ptr<ControllerDevice> controller)
{
  // Unassociate it from any other instances
  for (auto instance : GetAllInstances())
    instance->UnassociateController(controller);

  // Make association
  associatedControllers[controller->GetId()] = controller;

  // Raise
  OnControllerSelect.Invoke(controller);
}

void UIControllerSelectable::UnassociateController(std::shared_ptr<ControllerDevice> controller)
{
  // If it's not associated, ignore
  if (associatedControllers.count(controller->GetId()) == 0)
    return;

  // Remove association
  associatedControllers.erase(controller->GetId());

  // Raise
  OnControllerUnselect.Invoke(controller);
}

void UIControllerSelectable::Update(float deltaTime)
{
  // Update timers
  for (auto &[controllerId, timer] : controllerCooldowns)
    timer.Update(deltaTime);
}

Timer &UIControllerSelectable::GetControllerTimer(std::shared_ptr<ControllerDevice> controller)
{
  // If it doesn't exist yet
  if (controllerCooldowns.count(controller->GetId()) == 0)
  {
    // Removes the timer for this controller
    auto destroyTimer = [controllerId = controller->GetId()]()
    {
      UIControllerSelectable::controllerCooldowns.erase(controllerId);
    };

    // Subscribe to destroy the timer when the controller dies
    controller->OnBeforeDestroy.AddListener("destroy-UIControllerSelectable-timer", destroyTimer);
  }

  // Return the timer
  return controllerCooldowns[controller->GetId()];
}
