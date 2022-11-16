#include "Collider.h"
#include "Game.h"
#include "Camera.h"
#include <memory>

using namespace std;

// Explicitly initialize box
Collider::Collider(GameObject &associatedObject, Rectangle box) : Component(associatedObject)
{
  SetBox(box);
}

// Use sprite's box
Collider::Collider(GameObject &associatedObject, shared_ptr<Sprite> sprite, Vector2 scale)
    : Collider(associatedObject,
               Rectangle(0, 0, sprite->GetWidth() * scale.x, sprite->GetHeight() * scale.y)) {}

// Use sprite's box
Collider::Collider(GameObject &associatedObject, shared_ptr<SpriteAnimator> animator, Vector2 scale)
    : Collider(associatedObject,
               Rectangle(0, 0, animator->GetFrameWidth() * scale.x, animator->GetFrameHeight() * scale.y)) {}

void Collider::SetBox(const Rectangle &newBox)
{
  box = newBox;

  maxVertexDistance = sqrt(box.width * box.width + box.height * box.height) / 2;
}

Rectangle Collider::GetBox() const { return box + gameObject.GetPosition(); }

void Collider::Start()
{
  // Announce to game state
  gameState.RegisterCollider(dynamic_pointer_cast<Collider>(GetShared()));
}

void Collider::Render()
{
  auto box = GetBox();
  auto camera = Camera::GetInstance();

  // Create an SDL point for each vertex
  SDL_Point vertices[5];

  // Starting and final points are top left
  vertices[0] = (SDL_Point)camera.WorldToScreen(box.TopLeft(gameObject.GetRotation()));
  vertices[1] = (SDL_Point)camera.WorldToScreen(box.BottomLeft(gameObject.GetRotation()));
  vertices[2] = (SDL_Point)camera.WorldToScreen(box.BottomRight(gameObject.GetRotation()));
  vertices[3] = (SDL_Point)camera.WorldToScreen(box.TopRight(gameObject.GetRotation()));
  vertices[4] = (SDL_Point)camera.WorldToScreen(box.TopLeft(gameObject.GetRotation()));

  // Get renderer
  auto renderer = Game::GetInstance().GetRenderer();

  // Set paint color to green
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);

  // Paint collider edges
  SDL_RenderDrawLines(renderer, vertices, 5);
}
