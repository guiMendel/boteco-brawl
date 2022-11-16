#ifndef __RIGIDBODY__
#define __RIGIDBODY__

#include "Vector2.h"
#include "GameObject.h"
#include "Component.h"


enum class RigidbodyType
{
  Static,
  Dynamic
};

class Rigidbody : public Component
{
public:
  Rigidbody(GameObject &associatedObject, RigidbodyType type);

  virtual ~Rigidbody() {}

  void Update(float deltaTime) override;

  // Velocity for each axis
  Vector2 velocity{0, 0};

  // What the type of this body is
  RigidbodyType type{RigidbodyType::Static};

  // Gravity modifier
  Vector2 gravityScale{1, 1};

private:
};

#endif