#include "Collider.h"
#include "Rigidbody.h"
#include "Game.h"
#include "Camera.h"
#include "Debug.h"

using namespace std;

const float Rigidbody::defaultAirFriction{0.0f};

// Modifier applied to trajectory rectangle thickness
const float trajectoryThicknessModifier{0.8f};

// Slack to give beginning of trajectory
const float trajectorySlack{0.8f};

float CollidersProjectionSize(Rigidbody &body, Vector2 normal);

Rigidbody::Rigidbody(GameObject &associatedObject, RigidbodyType type, float elasticity, float friction)
    : Component(associatedObject), elasticity(elasticity), friction(friction), type(type), lastPosition(gameObject.GetPosition()) {}

float Rigidbody::GetMass() const
{
  if (type == RigidbodyType::Static)
    return numeric_limits<float>::max();

  return mass;
}

float Rigidbody::GetInverseMass() const
{
  if (type == RigidbodyType::Static)
    return 0;

  return inverseMass;
}

void Rigidbody::PhysicsUpdate(float deltaTime)
{
  if (type == RigidbodyType::Dynamic)
    DynamicBodyUpdate(deltaTime);

  // Update collision sets
  oldCollidingBodies = collidingBodies;
  collidingBodies.clear();
  oldCollidingTriggerBodies = collidingTriggerBodies;
  collidingTriggerBodies.clear();
}

void Rigidbody::DynamicBodyUpdate(float deltaTime)
{
  // Apply gravity
  velocity += GetState()->physicsSystem.gravity * gravityScale * deltaTime;

  // Apply air friction
  velocity = PhysicsSystem::ApplyFriction(velocity, airFriction);

  // Update the last position variable
  lastPosition = gameObject.GetPosition();

  // Move according to velocity
  gameObject.Translate(velocity * deltaTime);

  // Set this frame's possibly calculated trajectory as outdated
  frameTrajectoryOutdated = true;
}

void Rigidbody::UseAutoMass(bool value)
{
  useAutoMass = value;

  // Calculate it
  DeriveMassFromColliders();
}

void Rigidbody::InternalSetMass(float newMass)
{
  mass = newMass;
  inverseMass = 1.0f / newMass;
}

void Rigidbody::SetMass(float newMass)
{
  if (useAutoMass)
  {
    cout << "WARNING: tried setting mass when useAutoMass is set to true" << endl;
    return;
  }

  InternalSetMass(newMass);
}

void Rigidbody::DeriveMassFromColliders()
{
  float totalMass = 0;

  for (auto collider : GetColliders())
  {
    totalMass += collider->GetMass();
  }

  InternalSetMass(totalMass);
}

// Gets the list of colliders associated with this body
vector<shared_ptr<Collider>> Rigidbody::GetColliders() const
{
  return GetState()->physicsSystem.ValidateColliders(gameObject.id);
}

void Rigidbody::ApplyImpulse(Vector2 impulse)
{
  if (type == RigidbodyType::Static)
    return;

  velocity += impulse * inverseMass;
}

bool Rigidbody::IsCollidingWith(int id)
{
  return collidingBodies.count(id) > 0;
}

bool Rigidbody::WasCollidingWith(int id)
{
  return oldCollidingBodies.count(id) > 0;
}

bool Rigidbody::IsCollidingTriggerWith(int id)
{
  return collidingTriggerBodies.count(id) > 0;
}

bool Rigidbody::WasCollidingTriggerWith(int id)
{
  return oldCollidingTriggerBodies.count(id) > 0;
}

void Rigidbody::OnCollision(Collision::CollisionData collisionData)
{
  if (collisionData.other.expired())
    return;

  // Add to collision set
  collidingBodies.insert(collisionData.other.lock()->gameObject.id);
}

void Rigidbody::OnTriggerCollision(GameObject &other)
{
  collidingTriggerBodies.insert(other.id);
}

void Rigidbody::CalculateSmallestColliderDimension()
{
  smallestDimension = numeric_limits<float>::max();

  // For each collider
  for (auto collider : GetColliders())
  {
    auto colliderBox = collider->GetBox();

    // Compare
    smallestDimension = min(colliderBox.width, smallestDimension);
    smallestDimension = min(colliderBox.height, smallestDimension);
  }

  // Square it
  sqrSmallestDimension = smallestDimension * smallestDimension;
}

bool Rigidbody::ShouldUseContinuousDetection() const
{
  // Cut short if disabled
  if (continuousCollisions == false)
    return false;

  // Check that it's moved at least as much as it's smallest dimension
  auto sqrDistance = (gameObject.GetPosition() - lastPosition).SqrMagnitude();

  // Only use continuous if it's moved more than this
  return sqrDistance > sqrSmallestDimension;
}

pair<Rectangle, float> Rigidbody::GetFrameTrajectory()
{
  if (frameTrajectoryOutdated == false)
    return frameTrajectory;

  // Get displacement
  Vector2 displacement = gameObject.GetPosition() - lastPosition;

  // Get displacement normal
  Vector2 normal = displacement.Normalized().Rotated(M_PI / 2.0f);

  // Get colliders projection size on normal
  float projectionSize = CollidersProjectionSize(*this, normal) * trajectoryThicknessModifier;

  displacement.SetMagnitude(displacement.Magnitude() - trajectorySlack);

  // Store result
  // Place rectangle so that it's left face sits where the gameObject was
  frameTrajectory = make_pair(
      Rectangle(lastPosition + Vector2::Angled(displacement.Angle(), trajectorySlack) + displacement / 2,
                displacement.Magnitude(), projectionSize),
      displacement.Angle());

  frameTrajectoryOutdated = false;

  return frameTrajectory;
}

float CollidersProjectionSize(Rigidbody &body, Vector2 normal)
{
  // The biggest projection found
  float biggestProjection{numeric_limits<float>::lowest()};

  // The smallest projection found
  float smallestProjection{numeric_limits<float>::max()};

  // The bodies rotation
  float rotation = body.gameObject.GetRotation();

  // For each of it's colliders
  for (auto collider : body.GetColliders())
  {
    // Get collider's center
    Vector2 center = collider->GetBox().center;

    // For each of the collider's vertices
    for (auto vertex : collider->GetBox().Vertices(rotation))
    {
      // Get projection of vector from center to vertex
      float projectionSize = Vector2::Dot(vertex - center, normal);

      // Compare it
      biggestProjection = max(biggestProjection, projectionSize);
      smallestProjection = min(smallestProjection, projectionSize);
    }
  }

  // Return the the distance between each projection
  return biggestProjection - smallestProjection;
}

void Rigidbody::Render()
{
  auto [box, rotation] = frameTrajectory;

  Debug::DrawBox(box, rotation, Color::Blue());

  auto renderer = Game::GetInstance().GetRenderer();
  auto camera = Camera::GetMain();

  for (auto line : printLines)
  {
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_Point v[2]{(SDL_Point)camera->WorldToScreen(line.first), (SDL_Point)camera->WorldToScreen(line.second)};
    SDL_RenderDrawLines(renderer, v, 2);
  }

  if (printIntersectionPoint)
  {
    for (auto inter : intersectionPoints)
      Debug::DrawCircle(Circle{inter, 0.3});
  }
}

RigidbodyType Rigidbody::GetType() const { return type; }

void Rigidbody::SetType(RigidbodyType newType)
{
  if (type == newType)
    return;

  type = newType;
  GetState()->physicsSystem.UnregisterColliders(gameObject.id);

  // Re-register colliders
  for (auto collider : GetColliders())
    GetState()->physicsSystem.RegisterCollider(collider, gameObject.id);
}

bool Rigidbody::Raycast(float angle, float maxDistance, RaycastCollisionData &data)
{
  // Create a filter of this own object
  CollisionFilter filter;
  filter.ignoredObjects.insert(gameObject.id);

  return GetState()->physicsSystem.Raycast(gameObject.GetPosition(), angle, maxDistance, data, filter);
}

bool Rigidbody::Raycast(float angle, float maxDistance)
{
  RaycastCollisionData discardedData;
  return Raycast(angle, maxDistance, discardedData);
}

bool Rigidbody::ColliderCast(float angle, float maxDistance, RaycastCollisionData &data, float scaleColliders)
{
  // Create a filter of this own object
  CollisionFilter filter;
  filter.ignoredObjects.insert(gameObject.id);

  return GetState()->physicsSystem.ColliderCast(GetColliders(), gameObject.GetPosition(), angle, maxDistance, data, filter, scaleColliders);
}

bool Rigidbody::ColliderCast(float angle, float maxDistance, float scaleColliders)
{
  RaycastCollisionData discardedData;
  return ColliderCast(angle, maxDistance, discardedData, scaleColliders);
}
