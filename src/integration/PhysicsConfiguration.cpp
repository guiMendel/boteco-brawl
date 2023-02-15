#include "PhysicsSystem.h"
#include "PhysicsLayerHandler.h"
#include "BoxCollider.h"
#include "CircleCollider.h"
#include "Attack.h"
#include "Debug.h"

using namespace std;

#define RENDER_COLLIDER_OUTLINE

// Initial gravity
const Vector2 PhysicsSystem::initialGravity{0, 16};

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
#ifdef RENDER_COLLIDER_OUTLINE
  Debug::DrawBox(*RequirePointerCast<Rectangle>(DeriveShape()));
#endif
}

// Allows for debug rendering
void CircleCollider::Render()
{
#ifdef RENDER_COLLIDER_OUTLINE
  if (worldObject.GetName() == "Hitbox")
  {
    auto circle = *RequirePointerCast<Circle>(DeriveShape());
    auto attack = worldObject.RequireComponent<Attack>();

    float direction = GetSign(attack->worldObject.GetScale().x);

    Debug::DrawCircle(circle, Color::Pink());
    Debug::DrawPoint(circle.center, Color::Pink());

    if (attack->damage.impulse.type == AttackImpulse::Directed)
    {
      Debug::DrawArrow(circle.center,
                       circle.center + attack->damage.impulse.direction * circle.radius * Vector2(direction, 1),
                       Color::Cyan(),
                       circle.radius / 2);
    }

    else
    {
      Debug::DrawLine(circle.center + Vector2::AngledDegrees(-45, circle.radius / 2),
                      circle.center + Vector2::AngledDegrees(135, circle.radius / 2),
                      Color::Cyan());

      Debug::DrawLine(circle.center + Vector2::AngledDegrees(-135, circle.radius / 2),
                      circle.center + Vector2::AngledDegrees(45, circle.radius / 2),
                      Color::Cyan());
    }

    return;
  }

  Debug::DrawCircle(*RequirePointerCast<Circle>(DeriveShape()));
#endif
}
