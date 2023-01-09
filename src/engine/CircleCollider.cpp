#include "CircleCollider.h"
#include "Debug.h"

using namespace std;

CircleCollider::CircleCollider(GameObject &associatedObject, Circle circle, bool isTrigger, ColliderDensity density)
    : Collider(associatedObject, make_shared<Circle>(circle), isTrigger, density) {}

Circle CircleCollider::GetCircle() const { return *dynamic_pointer_cast<Circle>(shape); }

void CircleCollider::SetCircle(const Circle &circle) { shape = make_shared<Circle>(circle); }

shared_ptr<Shape> CircleCollider::CreateEmptyShape() const { return make_shared<Circle>(); }

// Allows for debug rendering
void CircleCollider::Render()
{
    Debug::DrawCircle(*dynamic_pointer_cast<Circle>(DeriveShape()));
}
