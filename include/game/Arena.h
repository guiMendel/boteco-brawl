#ifndef __ARENA__
#define __ARENA__

#include "WorldObject.h"
#include "Component.h"
#include "SpriteRenderer.h"

class Arena : public Component
{
public:
  Arena(WorldObject &associatedObject, float width, float height);

  Arena(WorldObject &associatedObject, std::shared_ptr<SpriteRenderer> backgroundRenderer);

  virtual ~Arena() {}
  void Render() override;

  RenderLayer GetRenderLayer() override { return RenderLayer::Debug; }

  // Arena size's width
  const float width;

  // Arena size's height
  const float height;

private:
  // Reference to background sprite renderer
  std::weak_ptr<SpriteRenderer> weakBackgroundRenderer;
};

#endif