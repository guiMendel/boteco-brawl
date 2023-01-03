#include "Collider.h"
#include "Game.h"
#include "Camera.h"
#include "Animation.h"
#include "Debug.h"
#include <memory>

using namespace std;

Rectangle RectangleFromAnimator(Animator &animator, Vector2 scale)
{
  Animation &defaultAnimation = animator.GetAnimation(animator.defaultAnimation);
  return Rectangle(
      0,
      0,
      defaultAnimation[0].GetSprite()->GetWidth() * scale.x,
      defaultAnimation[0].GetSprite()->GetHeight() * scale.y);
}

// Explicitly initialize box
Collider::Collider(GameObject &associatedObject, Rectangle box, bool isTrigger, ColliderDensity density) : Component(associatedObject), isTrigger(isTrigger), density(density)
{
  SetBox(box);
}

// Use spriteRenderer's box
Collider::Collider(GameObject &associatedObject, shared_ptr<SpriteRenderer> spriteRenderer, bool isTrigger, ColliderDensity density, Vector2 scale)
    : Collider(associatedObject,
               Rectangle(0, 0, spriteRenderer->sprite->GetWidth() * scale.x, spriteRenderer->sprite->GetHeight() * scale.y), isTrigger, density) {}

// Use spriteRenderer's box
Collider::Collider(GameObject &associatedObject, shared_ptr<Animator> animator, bool isTrigger, ColliderDensity density, Vector2 scale)
    : Collider(associatedObject,
               RectangleFromAnimator(*animator, scale), isTrigger, density) {}

Collider::Collider(GameObject &associatedObject, shared_ptr<Collider> other, bool isTrigger, ColliderDensity density, Vector2 scale)
    : Collider(associatedObject, Rectangle({0, 0}, other->GetBox().width * scale.x, other->GetBox().height * scale.y), isTrigger, density) {}

void Collider::SetBox(const Rectangle &newBox)
{
  box = newBox;

  maxVertexDistance = sqrt(box.width * box.width + box.height * box.height) / 2;
}

Rectangle Collider::GetBox() const { return box + gameObject.GetPosition(); }

void Collider::RegisterToState()
{
  // Id of gameObject on which to subscribe this collider
  int ownerId = isTrigger ? gameObject.id : -1;

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

void Collider::Render()
{
  Debug::DrawBox(GetBox(), gameObject.GetRotation());
}

float Collider::GetArea() const
{
  return box.width * box.height;
}

float Collider::GetDensity() const
{
  return static_cast<int>(density);
}

shared_ptr<Rigidbody> Collider::RequireRigidbody() const
{
  if (rigidbodyWeak.expired())
    throw runtime_error("Collider required a rigidbody but didn't have one");

  return rigidbodyWeak.lock();
}
