#ifndef __HEAT__
#define __HEAT__

#include "Component.h"
#include "Damage.h"
#include "Rigidbody.h"
#include "Movement.h"

struct Heat : public Component
{
public:
  // Max value heat can assume
  static const float maxHeat;

  Heat(GameObject &associatedObject, float armor);
  virtual ~Heat() {}

  void Awake() override;
  void Start() override;

  // Applies the given damage to the current heat level and applies it's impulse to the body
  void TakeDamage(const Damage &damage);

  // Gets current level of heat
  float GetHeat() const;

  // Get current armor value
  float GetArmor() const;

  // Set armor
  // Values must be >= 1, and usually go as high as 10
  void SetArmor(float value);

private:
  static const float inverseMaxHeat;

  // Current level of heat
  float heat{0};

  // Modifier applied to any incoming heat damage
  float armor, inverseArmor;

  // Reference to body
  std::weak_ptr<Rigidbody> weakBody;
  std::weak_ptr<Movement> weakMovement;
};

#endif