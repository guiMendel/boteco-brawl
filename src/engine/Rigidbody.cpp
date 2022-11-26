#include "Collider.h"
#include "Rigidbody.h"

using namespace std;

float CollidersProjectionSize(Rigidbody &body, Vector2 normal);

Rigidbody::Rigidbody(GameObject &associatedObject, RigidbodyType type, float elasticity, float friction)
    : Component(associatedObject), type(type), elasticity(elasticity), friction(friction), lastPosition(gameObject.GetPosition()) {}

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

  // Update the last position variable
  lastPosition = gameObject.GetPosition();

  // Set this frame's possibly calculated trajectory as outdated
  frameTrajectoryOutdated = true;
}

void Rigidbody::DynamicBodyUpdate(float deltaTime)
{
  // Apply gravity
  velocity += gameState.physicsSystem.gravity * gravityScale * deltaTime;

  // cout << gameObject.GetName() << " velocity x: " << velocity.x << endl;

  // Move according to velocity
  gameObject.Translate(velocity * deltaTime);
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
  return gameState.physicsSystem.ValidateColliders(gameObject.id);
}

void Rigidbody::ApplyImpulse(Vector2 impulse)
{
  if (type == RigidbodyType::Static)
    return;

  // cout << "Impulse: " << (string)impulse << ", final velocity: " << (string)(velocity + impulse * inverseMass) << endl;

  velocity += impulse * inverseMass;
}

bool Rigidbody::IsCollidingWith(Rigidbody &otherBody)
{
  return collidingBodies.count(otherBody.gameObject.id) > 0;
}

bool Rigidbody::WasCollidingWith(Rigidbody &otherBody)
{
  return oldCollidingBodies.count(otherBody.gameObject.id) > 0;
}

void Rigidbody::OnCollision(SatCollision::CollisionData collisionData)
{
  // Add to collision set
  collidingBodies.insert(collisionData.other->gameObject.id);
}

void Rigidbody::CalculateSmallestColliderDimension()
{
  sqrSmallestDimension = numeric_limits<float>::max();

  // For each collider
  for (auto collider : GetColliders())
  {
    auto colliderBox = collider->GetBox();

    // Compare
    sqrSmallestDimension = min(colliderBox.width, sqrSmallestDimension);
    sqrSmallestDimension = min(colliderBox.height, sqrSmallestDimension);
  }

  // Square it
  sqrSmallestDimension *= sqrSmallestDimension;
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
  float projectionSize = CollidersProjectionSize(*this, normal);

  // Store result
  // Place rectangle so that it's left face sits where the gameObject is
  frameTrajectory = make_pair(
      Rectangle(gameObject.GetPosition() + displacement / 2,
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
    Vector2 center = collider->GetBox().Center();

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
