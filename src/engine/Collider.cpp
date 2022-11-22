#include "Collider.h"
#include "Game.h"
#include "Camera.h"
#include <memory>

using namespace std;

// Explicitly initialize box
Collider::Collider(GameObject &associatedObject, Rectangle box, bool isTrigger, ColliderDensity density) : Component(associatedObject), isTrigger(isTrigger), density(density)
{
  SetBox(box);
}

// Use sprite's box
Collider::Collider(GameObject &associatedObject, shared_ptr<Sprite> sprite, bool isTrigger, ColliderDensity density, Vector2 scale)
    : Collider(associatedObject,
               Rectangle(0, 0, sprite->GetWidth() * scale.x, sprite->GetHeight() * scale.y), isTrigger, density) {}

// Use sprite's box
Collider::Collider(GameObject &associatedObject, shared_ptr<SpriteAnimator> animator, bool isTrigger, ColliderDensity density, Vector2 scale)
    : Collider(associatedObject,
               Rectangle(0, 0, animator->GetFrameWidth() * scale.x, animator->GetFrameHeight() * scale.y), isTrigger, density) {}

void Collider::SetBox(const Rectangle &newBox)
{
  box = newBox;

  maxVertexDistance = sqrt(box.width * box.width + box.height * box.height) / 2;
}

Rectangle Collider::GetBox() const { return box + gameObject.GetPosition(); }

void Collider::Start()
{
  // Id of gameObject on which to subscribe this collider
  int ownerId = isTrigger ? gameObject.id : -1;

  // If not trigger, find the rigidbody
  if (isTrigger == false)
  {
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
  }

  // Subscribe, if managed to find a valid id
  if (ownerId >= 0)
  {
    gameState.physicsSystem.RegisterCollider(dynamic_pointer_cast<Collider>(GetShared()), ownerId);
  }
}

void Collider::Render()
{
  auto box = GetBox();
  auto camera = Camera::GetMain();

  // Create an SDL point for each vertex
  SDL_Point vertices[5];

  // Starting and final points are top left
  vertices[0] = (SDL_Point)camera->WorldToScreen(box.TopLeft(gameObject.GetRotation()));
  vertices[1] = (SDL_Point)camera->WorldToScreen(box.BottomLeft(gameObject.GetRotation()));
  vertices[2] = (SDL_Point)camera->WorldToScreen(box.BottomRight(gameObject.GetRotation()));
  vertices[3] = (SDL_Point)camera->WorldToScreen(box.TopRight(gameObject.GetRotation()));
  vertices[4] = (SDL_Point)camera->WorldToScreen(box.TopLeft(gameObject.GetRotation()));

  // Get renderer
  auto renderer = Game::GetInstance().GetRenderer();

  // Set paint color to green
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);

  // Paint collider edges
  SDL_RenderDrawLines(renderer, vertices, 5);
}

float Collider::GetArea() const
{
  return box.width * box.height;
}

float Collider::GetDensity() const
{
  return static_cast<int>(density);
}
