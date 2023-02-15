#ifndef __KIBA_LEMON_AOE__
#define __KIBA_LEMON_AOE__

#include "WorldComponent.h"
#include "ParticleEmitter.h"
#include "CharacterController.h"

struct KibaLemonAOE : public WorldComponent
{
public:
  // How long the effect should last
  static const float maxDuration;

  // Interval between hits
  static const float hitInterval;

  // Base damage of AOE
  static const float baseDamage;

  // Random factor applied to damage each hit
  static const std::pair<float, float> damageModifier;

  KibaLemonAOE(GameObject &associatedObject);
  virtual ~KibaLemonAOE() {}

  // Get new targets in range
  void OnTriggerCollisionEnter(TriggerCollisionData triggerData);

  // Targets leaving range
  void OnTriggerCollisionExit(TriggerCollisionData triggerData);

  // Add particles
  void Awake() override;

  // Apply damage to targets in range
  void Update(float) override;

  // Turns AOE on
  void Activate();

  // Gets if effect is currently on
  bool IsActive() const;

private:
  // Stops damaging
  void Deactivate();

  // Add a target
  void AddTarget(std::shared_ptr<CharacterController> target);

  // Remove a target
  void RemoveTarget(WorldObject &target);

  // Start modulation for a given target
  void StartModulation(WorldObject &target);

  // Stop modulation for a given target
  void StopModulation(WorldObject &target);

  // Milliseconds when activate was called
  int activateTime{0};

  // Whether is currently active
  bool active{false};

  // Emission particles
  std::weak_ptr<ParticleEmitter> weakParticles;

  // Which targets are currently in range (by world object id)
  std::unordered_map<int, std::weak_ptr<CharacterController>> targetsInRange;

  // Current direction of target color modulation (by world object id)
  std::unordered_map<int, int> targetModulationDirection;
};

#endif