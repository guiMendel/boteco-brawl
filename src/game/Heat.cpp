#include "Heat.h"
#include "Character.h"
#include "CharacterVFX.h"
#include "FallDeath.h"
#include "ParticleFX.h"

using namespace std;
using namespace Helper;

const float Heat::maxHeat{200};
const float Heat::inverseMaxHeat{1.0f / maxHeat};
const float Heat::blowLift{0.25f};

Heat::Heat(GameObject &associatedObject, float armor)
    : WorldComponent(associatedObject), heat(100) { SetArmor(armor); }

void Heat::Awake()
{
  weakBody = worldObject.RequireComponent<Rigidbody>();
  weakMovement = worldObject.RequireComponent<Movement>();
  weakStateManager = worldObject.RequireComponent<CharacterStateManager>();
  weakCharacterController = worldObject.RequireComponent<CharacterController>();
  weakTimeScaleManager = GetScene()->RequireFindComponent<TimeScaleManager>();
  weakShakeManager = GetScene()->RequireFindComponent<ShakeEffectManager>();
}

void Heat::Start()
{
  // On death, reset heat
  worldObject.RequireComponent<FallDeath>()->OnFall.AddListener("reset-heat", [this]()
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

  // When damage has impulse
  if (damage.impulse.magnitude != 0)
  {
    // Cancel any previous speed
    body->velocity = Vector2::Zero();

    // Get impulse multiplier from heat, a value from 1 to 100
    float heatMultiplier = 99 * heat * inverseMaxHeat + 1;

    // Apply multiplier
    damage.impulse.magnitude *= heatMultiplier;

    // Get base impulse
    auto impulse = damage.impulse.DeriveImpulse(worldObject.GetShared());

    // When grounded
    if (movement->IsGrounded())
      // Instantly lift target a little bit from the floor
      worldObject.Translate({0, -blowLift});

    // Apply impulse
    body->ApplyImpulse(impulse);

    // Get impulse x direction
    float impulseDirection = GetSign(impulse.x, 0);

    // Face inverse direction of impulse
    if (impulseDirection != 0)
      body->worldObject.localScale.x = -impulseDirection;
  }

  // MESSAGE << worldObject << " taking damage: " << damage.heatDamage << " heatDamage, " << impulse.Magnitude() << " impulse." << endl;
  // MESSAGE << "Inverse Armor: " << inverseArmor << ", Heat: " << heat << ", Inverse Max Heat: " << inverseMaxHeat << ", Heat multiplier: " << heatMultiplier << endl;
  // MESSAGE << "Resulting damage: " << inverseArmor * damage.heatDamage << ", Resulting velocity add: " << (impulse * heatMultiplier * body->GetInverseMass()).Magnitude() << endl;

  // Update damage heat
  damage.heatDamage *= inverseArmor;

  // Add heat
  SetHeat(heat + damage.heatDamage);

  // Hit stop effect with updated damage
  float hitStopDuration = TriggerHitEffect(damage);

  // Particle effect
  worldObject.RequireComponent<CharacterVFX>()->PlaySparks({0, 0}, {0.01, 0.001});

  OnTakeDamage.Invoke(damage, hitStopDuration);
}

float Heat::TriggerHitEffect(Damage damage)
{
  // An effect needs impulse (or some hit stop) AND a character author
  if ((damage.impulse.magnitude == 0 && damage.minHitStop == 0) || damage.weakAuthor.expired())
    return 0;

  IF_LOCK(damage.weakAuthor, author)
  {
    if (author->GetComponent<Character>() == nullptr)
      return 0;
  }

  // Multiplier to apply to impulse to get duration
  static const float impulseFactor{0.005};
  static const float maxDuration{1.5};

  LOCK(weakTimeScaleManager, timeScaleManager);
  LOCK(weakShakeManager, shakeManager);

  // Calculate duration
  float duration = Clamp(impulseFactor * damage.impulse.magnitude, damage.minHitStop, maxDuration);

  // Apply to self
  timeScaleManager->AlterTimeScale(worldObject.GetShared(), 0.00001, duration);

  // Apply to attack author
  IF_LOCK(damage.weakAuthor, author)
  {
    timeScaleManager->AlterTimeScale(author, 0.00001, duration);
  }

  auto impulse = damage.impulse.DeriveImpulse(worldObject.GetShared());

  if (damage.impulse.magnitude > 11)
    // Apply shaking
    shakeManager->Shake(
        worldObject.GetShared(),
        impulse.Angle(),
        {log10f(damage.impulse.magnitude - 10) / 2, 0},
        {0.2, 0.01},
        duration, 0);

  return duration;
}

void Heat::OnCollisionEnter(Collision::Data)
{
  LOCK(weakStateManager, stateManager);

  // Check if bouncing OR not already recovering
  if (stateManager->IsBouncing() == false || stateManager->HasState(RECOVERING_STATE))
    return;

  LOCK(weakCharacterController, characterController);
  LOCK(weakBody, body);

  // Take damage from collision, proportional to current speed
  float speedFactor = pow(body->velocity.SqrMagnitude(), 0.25f);

  Damage collisionDamage{speedFactor};

  // Add some hit stop too
  collisionDamage.minHitStop = speedFactor / 20;
  collisionDamage.stunTime = 0.5;

  characterController->TakeHit(collisionDamage, false);
}

void Heat::SetHeat(float newValue)
{
  if (heat == newValue)
    return;

  float oldValue = heat;

  heat = newValue;

  OnHeatChange.Invoke(newValue, oldValue);
}
