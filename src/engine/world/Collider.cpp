#include "Collider.h"
#include "Game.h"
#include "Camera.h"
#include "Animation.h"
#include "Debug.h"
#include <memory>

using namespace std;

Collider::Collider(GameObject &associatedObject, shared_ptr<Shape> shape, bool isTrigger, ColliderDensity density)
    : WorldComponent(associatedObject), isTrigger(isTrigger), density(density), shape(shape) {}

void Collider::RegisterToScene()
{
  // Id of worldObject on which to subscribe this collider
  ownerId = isTrigger ? worldObject.id : -1;

  // Object to inspect for a rigidbody
  shared_ptr<WorldObject> inspectingObject = worldObject.GetShared();

  // While it isn't null, check if it has a rigidbody
  while (inspectingObject != nullptr)
  {
    // Check it
    auto rigidbody = inspectingObject->GetComponent<Rigidbody>();

    // If it's not null, then it's the one
    if (rigidbody != nullptr)
    {
      ownerId = rigidbody->worldObject.id;
      rigidbodyWeak = rigidbody;
      break;
    }

    // Check next in line
    inspectingObject = inspectingObject->GetParent();
  }

  // IF_LOCK(rigidbodyWeak, body)
  // {
  //   cout << "Collider " << worldObject.GetName() << " has body in " << body->worldObject.GetName() << endl;
  // }
  // else cout << "Collider " << worldObject.GetName() << " has no body" << endl;

  // Subscribe, if managed to find a valid id
  if (ownerId >= 0)
    GetScene()->physicsSystem.RegisterCollider(RequirePointerCast<Collider>(GetShared()), ownerId);
  else
    cout << "WARNING: Object " << worldObject.GetName() << " has a non-trigger collider, but has no Rigidbody attached" << endl;
}

float Collider::GetDensity() const
{
  return static_cast<int>(density);
}

float Collider::GetMass() const
{
  return GetDensity() * shape->GetArea();
}

shared_ptr<Rigidbody> Collider::RequireRigidbody() const
{
  if (rigidbodyWeak.expired())
    throw runtime_error("Collider required a rigidbody but didn't have one");

  return rigidbodyWeak.lock();
}

shared_ptr<Shape> Collider::DeriveShape() const
{
  // Get a new shared ptr and shape
  auto shapeCopy = CopyShape();

  // Get scale
  Vector2 scale = worldObject.GetScale();

  // Apply scale
  shapeCopy->Scale(scale.GetAbsolute());

  // Handle negative scales
  if (scale.x < 0)
  {
    // Invert position offset
    shapeCopy->center.x = -shapeCopy->center.x;

    // Rotate to mirror around y-axis
    float mirrorAngle = M_PI / 2 * (sin(shapeCopy->rotation) < 0 ? -1 : 1);
    shapeCopy->Rotate(2 * (mirrorAngle - shapeCopy->rotation));
  }

  if (scale.y < 0)
  {
    // Invert position offset
    shapeCopy->center.y = -shapeCopy->center.y;

    // Rotate to mirror around x-axis
    float mirrorAngle = cos(shapeCopy->rotation) < 0 ? -M_PI : 0;
    shapeCopy->Rotate(2 * (mirrorAngle - shapeCopy->rotation));
  }

  // Pivot around worldObject according to it's rotation
  shapeCopy->center = worldObject.GetPosition().Pivot(shapeCopy->center, worldObject.GetRotation());

  // Apply position & rotation offsets with worldObject's values
  shapeCopy->Rotate(worldObject.GetRotation());
  shapeCopy->Displace(worldObject.GetPosition());

  return shapeCopy;
}

int Collider::GetOwnerId() const { return ownerId; }

shared_ptr<WorldObject> Collider::GetOwner() const
{
  return GetScene()->RequireWorldObject(GetOwnerId());
}

void Collider::OnBeforeDestroy()
{
  worldObject.HandleColliderDestruction(RequirePointerCast<Collider>(GetShared()));
}
