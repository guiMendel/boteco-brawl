#include "BoxCollider.h"
#include "Sprite.h"
#include "Animation.h"
#include "Debug.h"

using namespace std;

Rectangle RectangleFromAnimator(Animator &animator, Vector2 scale)
{
  Animation &defaultAnimation = animator.GetAnimation(animator.defaultAnimation);
  auto sprite = defaultAnimation[0].GetSprite();

  return Rectangle({0, 0}, sprite->GetWidth() * scale.x, sprite->GetHeight() * scale.y);
}
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
          animator->GetAnimation(animator->defaultAnimation)[0].GetSprite(),
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
  return *dynamic_pointer_cast<Rectangle>(shape);
}

void BoxCollider::SetBox(const Rectangle &box) { shape = make_shared<Rectangle>(box); }

shared_ptr<Shape> BoxCollider::CopyShape() const { return make_shared<Rectangle>(GetBox()); }

// Allows for debug rendering
void BoxCollider::Render()
{
  Debug::DrawBox(*dynamic_pointer_cast<Rectangle>(DeriveShape()));
}
