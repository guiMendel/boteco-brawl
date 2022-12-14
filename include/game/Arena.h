#ifndef __ARENA__
#define __ARENA__

#include "GameObject.h"
#include "Component.h"
#include "SpriteRenderer.h"

class Arena : public Component
{
public:
  Arena(GameObject &associatedObject, std::shared_ptr<SpriteRenderer> backgroundRenderer);
  virtual ~Arena() {}

  // Get's the background renderer
  std::shared_ptr<SpriteRenderer> GetBackgroundRenderer() const;

  // Arena size's width
  const float width;

  // Arena size's height
  const float height;

private:
  // Reference to background sprite renderer
  std::weak_ptr<SpriteRenderer> weakBackgroundRenderer;
};

#endif