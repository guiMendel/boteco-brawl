#include "Heat.h"
#include "FallOffDeath.h"
#include "ParticleFX.h"

using namespace std;
using namespace Helper;

const float Heat::maxHeat{200};
const float Heat::inverseMaxHeat{1.0f / maxHeat};

void PlayHitParticles(Vector2 source);

Heat::Heat(GameObject &associatedObject, float armor)
    : Component(associatedObject) { SetArmor(armor); }

void Heat::Awake()
{
  weakBody = gameObject.RequireComponent<Rigidbody>();
  weakMovement = gameObject.RequireComponent<Movement>();
  weakTimeScaleManager = GetState()->RequireObjectOfType<TimeScaleManager>();
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

void Heat::TakeDamage(Damage damage)
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

  // Update damage impulse
  damage.impulse *= heatMultiplier;

  // Apply impulse
  body->ApplyImpulse(damage.impulse);

  // Get impulse x direction
  float impulseDirection = GetSign(damage.impulse.x, 0);

  // Face inverse direction of impulse
  if (impulseDirection != 0)
    body->gameObject.localScale.x = -impulseDirection;

  // Update damage heat
  damage.heatDamage *= inverseArmor;

  // Add heat
  heat += damage.heatDamage;

  // Hit stop effect with updated damage
  TriggerHitStop(damage);

  // Particle effect
  PlayHitParticles(gameObject.GetPosition());
}

void Heat::TriggerHitStop(Damage damage)
{
  // Multiplier to apply to impulse to get duration
  static const float impulseFactor{0.005};
  static const float maxDuration{1.5};

  LOCK(weakTimeScaleManager, timeScaleManager);

  // Calculate duration
  float duration = min(impulseFactor * damage.impulse.Magnitude(), maxDuration);

  // Apply to self
  timeScaleManager->AlterTimeScale(gameObject.GetShared(), 0, duration);

  // Apply to attack author
  IF_LOCK(damage.weakAuthor, author)
  {
    timeScaleManager->AlterTimeScale(author, 0, duration);
  }
}

void PlayHitParticles(Vector2 source)
{
  // Get random angle
  float angleCenter = RandomRange(0, 2 * M_PI);

  // Arc of hit effect
  static const float effectArc{DegreesToRadians(30)};

  ParticleEmissionParameters params;
  params.color = {Color::Yellow(), Color::White()};
  params.speed = {-25, 25};
  params.frequency = {0.005, 0.0005};
  params.lifetime = {0.01, 0.1};
  params.angle = {angleCenter + effectArc / 2, angleCenter - effectArc / 2};

  ParticleFX::EffectAt(source, 0.1, 0.05, params, 0.2);
}
