#include "BoxCollider.h"
#include "Sprite.h"
#include "Animation.h"
#include "Debug.h"

using namespace std;

// Explicitly initialize box
BoxCollider::BoxCollider(GameObject &associatedObject, Rectangle box, bool isTrigger, ColliderDensity density)
    : Collider(associatedObject, make_shared<Rectangle>(box), isTrigger, density) {}

// Use sprite's box
BoxCollider::BoxCollider(
    GameObject &associatedObject, shared_ptr<Sprite> sprite, bool isTrigger, ColliderDensity density, Vector2 scale)
    : BoxCollider(
          associatedObject,
          Rectangle({0, 0}, sprite->GetWidth() * scale.x, sprite->GetHeight() * scale.y),
          isTrigger,
          density) {}

// Use sprite animator's initial animation sprite size
BoxCollider::BoxCollider(
    GameObject &associatedObject, shared_ptr<Animator> animator, bool isTrigger, ColliderDensity density, Vector2 scale)
    : BoxCollider(
          associatedObject,
          animator->BuildAnimation(animator->defaultAnimation)->GetFrame(0).GetSprite(),
          isTrigger,
          density,
          scale) {}

// Use other BoxCollider::BoxCollider's box
BoxCollider::BoxCollider(
    GameObject &associatedObject, shared_ptr<BoxCollider> other, bool isTrigger, ColliderDensity density, Vector2 scale)
    : BoxCollider(
          associatedObject,
          Rectangle({0, 0}, other->GetBox().width * scale.x, other->GetBox().height * scale.y),
          isTrigger,
          density) {}

Rectangle BoxCollider::GetBox() const
{
  return *RequirePointerCast<Rectangle>(shape);
}

void BoxCollider::SetBox(const Rectangle &box) { shape = make_shared<Rectangle>(box); }

shared_ptr<Shape> BoxCollider::CopyShape() const { return make_shared<Rectangle>(GetBox()); }
