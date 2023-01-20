#ifndef __ATTACK__
#define __ATTACK__

// Name of gameObject which will contain this component
#define ATTACK_OBJECT "Hitbox"

#include "Component.h"
#include "Damage.h"
#include "Character.h"

class CharacterController;

struct Attack : public Component
{
public:
  // Raised when attack connects
  EventI<std::shared_ptr<CharacterController>> OnConnect;

  // Initializes with the attack damage's parameters and an optional hit cooldown, which if set allows for multi-hits
  Attack(GameObject &associatedObject, DamageParameters damage, float hitSecondsCooldown = -1);
  virtual ~Attack() {}

  void Awake() override;
  void OnTriggerCollisionEnter(TriggerCollisionData) override;

  // Apply attack to a given character's controller
  void Land(std::shared_ptr<CharacterController> targetController);

  // Damage params
  DamageParameters damage;

private:
  // Gets the damage struct for this attack
  Damage GetDamage() const;

  // Milliseconds that must pass before a same target can be hit by this attack again
  // Negative values mean they can never be hit again
  float hitCooldown;

  // Ids of controllers which were already attacked, mapped to the time of the attack in ms
  std::unordered_map<int, int> struckTargetsTime;

  // Reference to character
  std::weak_ptr<Character> weakCharacter;
};

#endif