#include "Collider.h"
#include "Game.h"
#include "Camera.h"
#include "Animation.h"
#include "Debug.h"
#include <memory>

using namespace std;

Collider::Collider(GameObject &associatedObject, shared_ptr<Shape> shape, bool isTrigger, ColliderDensity density)
    : Component(associatedObject), isTrigger(isTrigger), density(density), shape(shape) {}

void Collider::RegisterToState()
{
  // Id of gameObject on which to subscribe this collider
  ownerId = isTrigger ? gameObject.id : -1;

  // Object to inspect for a rigidbody
  shared_ptr<GameObject> inspectingObject = gameObject.GetShared();

  // While it isn't null, check if it has a rigidbody
  while (inspectingObject != nullptr)
  {
    // Check it
    auto rigidbody = inspectingObject->GetComponent<Rigidbody>();

    // If it's not null, then it's the one
    if (rigidbody != nullptr)
    {
      ownerId = rigidbody->gameObject.id;
      rigidbodyWeak = rigidbody;
      break;
    }

    // Check next in line
    inspectingObject = inspectingObject->GetParent();
  }

  // IF_LOCK(rigidbodyWeak, body)
  // {
  //   cout << "Collider " << gameObject.GetName() << " has body in " << body->gameObject.GetName() << endl;
  // }
  // else cout << "Collider " << gameObject.GetName() << " has no body" << endl;

  // Subscribe, if managed to find a valid id
  if (ownerId >= 0)
    GetState()->physicsSystem.RegisterCollider(dynamic_pointer_cast<Collider>(GetShared()), ownerId);
  else
    cout << "WARNING: Object " << gameObject.GetName() << " has a non-trigger collider, but has no Rigidbody attached" << endl;
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
  Vector2 scale = gameObject.GetScale();

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

  // Pivot around gameObject according to it's rotation
  shapeCopy->center = gameObject.GetPosition().Pivot(shapeCopy->center, gameObject.GetRotation());

  // Apply position & rotation offsets with gameObject's values
  shapeCopy->Rotate(gameObject.GetRotation());
  shapeCopy->Displace(gameObject.GetPosition());

  return shapeCopy;
}

int Collider::GetOwnerId() const { return ownerId; }

shared_ptr<GameObject> Collider::GetOwner() const
{
  return GetState()->GetObject(GetOwnerId());
}
