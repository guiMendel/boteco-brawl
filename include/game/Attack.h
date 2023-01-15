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
  Attack(GameObject &associatedObject, DamageParameters damage);
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

  // Ids of controllers which were already attacked
  std::unordered_set<int> struckControllerIds;

  // Reference to character
  std::weak_ptr<Character> weakCharacter;
};

#endif