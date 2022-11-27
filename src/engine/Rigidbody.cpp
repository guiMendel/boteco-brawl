#include "Collider.h"
#include "Rigidbody.h"
#include "Game.h"
#include "Camera.h"

using namespace std;

const float Rigidbody::defaultAirFriction{0.1f};

// Modifier applied to trajectory rectangle thickness
const float trajectoryThicknessModifier{0.8f};

// Slack to give beginning of trajectory
const float trajectorySlack{0.8f};

void DrawCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius);
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
}

void Rigidbody::DynamicBodyUpdate(float deltaTime)
{
  // Apply gravity
  velocity += gameState.physicsSystem.gravity * gravityScale * deltaTime;

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
  return gameState.physicsSystem.ValidateColliders(gameObject.id);
}

void Rigidbody::ApplyImpulse(Vector2 impulse)
{
  if (type == RigidbodyType::Static)
    return;

  velocity += impulse * inverseMass;
}

bool Rigidbody::IsCollidingWith(Collider &other)
{
  return collidingBodies.count(other.gameObject.id) > 0;
}

bool Rigidbody::WasCollidingWith(Collider &other)
{
  return oldCollidingBodies.count(other.gameObject.id) > 0;
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

void Rigidbody::Render()
{
  // if (printDebug == false)
  //   return;

  // auto [box, rotation] = frameTrajectory;
  // auto camera = Camera::GetMain();

  // // Create an SDL point for each vertex
  // SDL_Point vertices[5];

  // // Starting and final points are top left
  // vertices[0] = (SDL_Point)camera->WorldToScreen(box.TopLeft(rotation));
  // vertices[1] = (SDL_Point)camera->WorldToScreen(box.BottomLeft(rotation));
  // vertices[2] = (SDL_Point)camera->WorldToScreen(box.BottomRight(rotation));
  // vertices[3] = (SDL_Point)camera->WorldToScreen(box.TopRight(rotation));
  // vertices[4] = (SDL_Point)camera->WorldToScreen(box.TopLeft(rotation));

  // // Get renderer
  // auto renderer = Game::GetInstance().GetRenderer();

  // // Set paint color to blu
  // SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);

  // // Paint collider edges
  // SDL_RenderDrawLines(renderer, vertices, 5);

  // for (auto line : printLines)
  // {
  //   SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
  //   SDL_Point v[2]{(SDL_Point)camera->WorldToScreen(line.first), (SDL_Point)camera->WorldToScreen(line.second)};
  //   SDL_RenderDrawLines(renderer, v, 2);
  // }

  // if (printIntersectionPoint)
  // {
  //   for (auto inter : intersectionPoints)
  //   {
  //     Vector2 point = camera->WorldToScreen(inter);
  //     DrawCircle(renderer, point.x, point.y, 7);
  //   }
  // }
}

RigidbodyType Rigidbody::GetType() const { return type; }

void Rigidbody::SetType(RigidbodyType newType)
{
  if (type == newType)
    return;

  type = newType;
  gameState.physicsSystem.UnregisterColliders(gameObject.id);

  // Re-register colliders
  for (auto collider : GetColliders())
    gameState.physicsSystem.RegisterCollider(collider, gameObject.id);
}

//
//
//
//
//
//
//
void DrawCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
  const int32_t diameter = (radius * 2);

  int32_t x = (radius - 1);
  int32_t y = 0;
  int32_t tx = 1;
  int32_t ty = 1;
  int32_t error = (tx - diameter);

  while (x >= y)
  {
    //  Each of the following renders an octant of the circle
    SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
    SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
    SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
    SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
    SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
    SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
    SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
    SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

    if (error <= 0)
    {
      ++y;
      error += ty;
      ty += 2;
    }

    if (error > 0)
    {
      --x;
      tx += 2;
      error += (tx - diameter);
    }
  }
}