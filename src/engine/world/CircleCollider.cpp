#include "CircleCollider.h"
#include "Debug.h"

using namespace std;

CircleCollider::CircleCollider(GameObject &associatedObject, Circle circle, bool isTrigger, ColliderDensity density)
    : Collider(associatedObject, make_shared<Circle>(circle), isTrigger, density) {}

Circle CircleCollider::GetCircle() const { return *RequirePointerCast<Circle>(shape); }

void CircleCollider::SetCircle(const Circle &circle) { shape = make_shared<Circle>(circle); }

shared_ptr<Shape> CircleCollider::CopyShape() const { return make_shared<Circle>(GetCircle()); }
