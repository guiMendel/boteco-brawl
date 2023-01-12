#include "PhysicsSystem.h"
#include "PhysicsLayerHandler.h"
#include "BoxCollider.h"
#include "CircleCollider.h"
#include "Debug.h"

using namespace std;

// Initial gravity
const Vector2 PhysicsSystem::initialGravity{0, 14};

void PhysicsLayerHandler::InitializeCollisionMatrix()
{
  // Characters don't collide (normally) with each other
  Disable(PhysicsLayer::Character, PhysicsLayer::Character);

  // But their repel boxes do collide only with characters
  DisableAll(PhysicsLayer::CharacterRepelBox);
  Enable(PhysicsLayer::CharacterRepelBox, PhysicsLayer::Character);

  // Platform detectors only collide with platforms
  DisableAll(PhysicsLayer::CharacterPlatformDrop);
  Enable(PhysicsLayer::CharacterPlatformDrop, PhysicsLayer::Platform);

  // Hitboxes can only hit characters
  DisableAll(PhysicsLayer::Hitbox);
  Enable(PhysicsLayer::Hitbox, PhysicsLayer::Character);
}

// Allows for debug rendering
void BoxCollider::Render()
{
  Debug::DrawBox(*dynamic_pointer_cast<Rectangle>(DeriveShape()));
}

// Allows for debug rendering
void CircleCollider::Render()
{
  if (gameObject.GetName() == "Hitbox")
  {
    auto circle = *dynamic_pointer_cast<Circle>(DeriveShape());
    Debug::DrawCircle(circle, Color::Pink());
    Debug::DrawPoint(circle.center, Color::Pink());
    return;
  }

  Debug::DrawCircle(*dynamic_pointer_cast<Circle>(DeriveShape()));
}
