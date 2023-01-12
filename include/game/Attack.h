#ifndef __ATTACK__
#define __ATTACK__

// Name of gameObject which will contain this component
#define ATTACK_OBJECT "Hitbox"

#include "Component.h"
#include "Damage.h"
#include "Character.h"

class Heat;

struct Attack : public Component
{
public:
  Attack(GameObject &associatedObject, float damageModifier, Vector2 impulse);
  virtual ~Attack() {}

  void Awake() override;
  void OnTriggerCollisionEnter(TriggerCollisionData) override;

  // Apply attack to a given character's heat
  void Land(std::shared_ptr<Heat> targetHeat);

  // Attack damage modifier
  float damageModifier;

  // Attack impulse
  Vector2 impulse;

private:
  // Gets the damage struct for this attack
  Damage GetDamage() const;

  // Ids of heats which were already attacked
  std::unordered_set<int> struckHeatIds;

  // Reference to character
  std::weak_ptr<Character> weakCharacter;
};

#endif