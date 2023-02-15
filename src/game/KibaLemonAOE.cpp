#include "KibaLemonAOE.h"
#include "FallDeath.h"
#include "BoxCollider.h"

#define EMISSION_STARTING_FREQUENCY make_pair(0.001, 0.005)

#define DURATION_TIMER "lemon-aoe-duration"
#define HIT_TIMER "lemon-aoe-hit"
#define MODULATION_SPEED 700
#define MIN_MODULATION 100
#define MAX_MODULATION 200

using namespace std;

const float KibaLemonAOE::maxDuration{8};
const float KibaLemonAOE::hitInterval{0.3};
const float KibaLemonAOE::baseDamage{0.3};
const pair<float, float> KibaLemonAOE::damageModifier{0.7, 2.3};

KibaLemonAOE::KibaLemonAOE(GameObject &associatedObject) : WorldComponent(associatedObject) {}

void KibaLemonAOE::Awake()
{
  // Get collider box
  Circle area{worldObject.GetParent()->RequireComponent<BoxCollider>()->GetBox().GetMaxDimension() / 2};

  // Add particles
  auto particles = worldObject.AddComponent<ParticleEmitter>(RenderLayer::VFX, make_unique<Circle>(area), true);
  particles->irradiateParticles = true;
  weakParticles = particles;

  // Diminish emission and speed with time
  auto diminish = [this](ParticleEmissionParameters &emission, float)
  {
    // Get how much time has elapsed
    float elapsedTime = (SDL_GetTicks() - activateTime) / 1000;

    // Set parameters
    emission.frequency = {
        Lerp(EMISSION_STARTING_FREQUENCY.first, EMISSION_STARTING_FREQUENCY.first * 15, elapsedTime / maxDuration),
        Lerp(EMISSION_STARTING_FREQUENCY.second, EMISSION_STARTING_FREQUENCY.second * 15, elapsedTime / maxDuration)};
  };

  particles->emission.color = {Color(173, 250, 0, 160), Color(231, 255, 218, 160)};
  particles->emission.frequency = EMISSION_STARTING_FREQUENCY;
  particles->emission.gravityModifier = {Vector2::Down(0.03), Vector2::Down(0.07)};
  particles->emission.lifetime = {0.5, 1.2};
  particles->emission.speed = {1.5, 3};
  particles->emissionEvolution = diminish;
  particles->emitOnStart = false;

  // Deactivate on kiba death
  worldObject.GetParent()->RequireComponent<FallDeath>()->OnFall.AddListener("deactivate-lime-aoe", [this]()
                                                                             { Deactivate(); });
}

void KibaLemonAOE::OnTriggerCollisionEnter(TriggerCollisionData triggerData)
{
  // Get character controller
  auto targetController = Lock(triggerData.weakOther)->worldObject.GetComponent<CharacterController>();

  // Ignore if not present
  if (targetController == nullptr)
    return;

  // Add as new target
  AddTarget(targetController);
}

void KibaLemonAOE::OnTriggerCollisionExit(TriggerCollisionData triggerData)
{
  // Remove target
  RemoveTarget(Lock(triggerData.weakOther)->worldObject);
}

void KibaLemonAOE::AddTarget(std::shared_ptr<CharacterController> target)
{
  targetsInRange[target->worldObject.id] = target;
  targetModulationDirection[target->worldObject.id] = -1;
}

void KibaLemonAOE::RemoveTarget(WorldObject &target)
{
  // Remove from map
  targetsInRange.erase(target.id);
  targetModulationDirection.erase(target.id);

  // Reset modulation
  StopModulation(target);
}

void KibaLemonAOE::StartModulation(WorldObject &target)
{
  auto targetRenderer = target.RequireComponent<SpriteRenderer>();

  auto currentColor = targetRenderer->GetColors().first;
  targetRenderer->SetColor(Color{MAX_MODULATION, 225, MAX_MODULATION, currentColor.alpha});

  targetModulationDirection.at(target.id) = -1;
}

void KibaLemonAOE::StopModulation(WorldObject &target)
{
  auto targetRenderer = target.RequireComponent<SpriteRenderer>();

  auto currentColor = targetRenderer->GetColors().first;
  targetRenderer->SetColor(Color{225, 225, 225, currentColor.alpha});
}

void KibaLemonAOE::Activate()
{
  // Record time
  activateTime = SDL_GetTicks();

  // Start emitting
  active = true;

  // Start timer
  worldObject.timer.Reset(DURATION_TIMER, -maxDuration);

  // Start modulation of objects already in range
  for (auto target : WeakMapToVector(targetsInRange))
    StartModulation(target->worldObject);

  Lock(weakParticles)->StartEmission();
}

void KibaLemonAOE::Deactivate()
{
  active = false;

  worldObject.timer.Stop(DURATION_TIMER);

  // Stop modulation of objects in range
  for (auto target : WeakMapToVector(targetsInRange))
    StopModulation(target->worldObject);

  Lock(weakParticles)->Stop();
}

void KibaLemonAOE::Update(float deltaTime)
{
  if (active == false)
    return;

  // Check timer
  if (worldObject.timer.Get(DURATION_TIMER) >= 0)
  {
    Deactivate();
    return;
  }

  // Damage everyone
  for (auto target : WeakMapToVector(targetsInRange))
  {
    // Modulate color
    auto targetRenderer = target->worldObject.RequireComponent<SpriteRenderer>();
    auto currentColor = targetRenderer->GetColors().first;

    int &direction = targetModulationDirection.at(target->worldObject.id);

    // Get new value for R & B channels
    int newValue = currentColor.red + direction * MODULATION_SPEED * deltaTime;

    // Check if should change direction
    if (newValue <= MIN_MODULATION || newValue >= MAX_MODULATION)
    {
      newValue = Clamp(newValue, MIN_MODULATION, MAX_MODULATION);
      direction = -direction;
    }

    targetRenderer->SetColor(Color{newValue, 225, newValue, currentColor.alpha});

    // Check if this hit is on cooldown
    if (target->worldObject.timer.Get(HIT_TIMER) < 0)
      return;

    // Get damage
    float damage = RandomRange(damageModifier) * baseDamage;

    target->TakeHit(Damage{damage}, false);

    // Start hit cooldown timer
    target->worldObject.timer.Reset(HIT_TIMER, -hitInterval);
  }
}

bool KibaLemonAOE::IsActive() const { return active; }
