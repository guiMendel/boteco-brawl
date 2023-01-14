#include "Heat.h"
#include "FallOffDeath.h"

using namespace std;
using namespace Helper;

const float Heat::maxHeat{200};
const float Heat::inverseMaxHeat{1.0f / maxHeat};

// How much gameObject gets lifted when struck with an attack while grounded

Heat::Heat(GameObject &associatedObject, float armor)
    : Component(associatedObject) { SetArmor(armor); }

void Heat::Awake()
{
  weakBody = gameObject.RequireComponent<Rigidbody>();
  weakMovement = gameObject.RequireComponent<Movement>();
}

void Heat::Start()
{
  // On death, reset heat
  gameObject.RequireComponent<FallOffDeath>()->OnDeath.AddListener("reset-heat", [this]()
                                                                   { heat = 0; });
}

// Gets current level of heat
float Heat::GetHeat() const { return heat; }

// Get current armor value
float Heat::GetArmor() const { return armor; }

// Set armor
void Heat::SetArmor(float value)
{
  Assert(value >= 1, "Armor values must be greater or equal to 1");
  armor = value;
  inverseArmor = 1.0f / value;
}

void Heat::TakeDamage(const Damage &damage)
{
  // Get body
  LOCK(weakBody, body);
  LOCK(weakMovement, movement);

  // Cancel any previous speed
  body->velocity = Vector2::Zero();

  // Get impulse multiplier from heat, a value from 1 to 100
  float heatMultiplier = 99 * heat * inverseMaxHeat + 1;

  cout << gameObject << " taking damage: " << damage.heatDamage << " heatDamage, " << Vector2(damage.impulse).Magnitude() << " impulse." << endl;
  cout << "Inverse Armor: " << inverseArmor << ", Heat: " << heat << ", Inverse Max Heat: " << inverseMaxHeat << ", Heat multiplier: " << heatMultiplier << endl;
  cout << "Resulting damage: " << inverseArmor * damage.heatDamage << ", Resulting velocity add: " << (damage.impulse * heatMultiplier * body->GetInverseMass()).Magnitude() << endl;

  // When grounded
  if (movement->IsGrounded())
    // Instantly lift target a little bit from the floor
    gameObject.Translate({0, -0.15});

  // Apply impulse
  body->ApplyImpulse(damage.impulse * heatMultiplier);

  // Get impulse x direction
  float impulseDirection = GetSign(damage.impulse.x, 0);

  // Face inverse direction of impulse
  if (impulseDirection != 0)
    body->gameObject.localScale.x = -impulseDirection;

  // Add heat
  heat += inverseArmor * damage.heatDamage;
}
