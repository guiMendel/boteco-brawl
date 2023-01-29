#ifndef __KEYBOARD_INPUT__
#define __KEYBOARD_INPUT__

#include "PlayerInput.h"

class KeyboardInput : public PlayerInput
{
public:
  KeyboardInput(GameObject &associatedObject);
  virtual ~KeyboardInput() {}

  void Update(float deltaTime) override;

private:
  // Gets input direction for this frame
  Vector2 GetInputDirection() const;
};

#endif