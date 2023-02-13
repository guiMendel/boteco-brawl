#ifndef __ARENA__
#define __ARENA__

#include "WorldObject.h"
#include "WorldComponent.h"
#include "SpriteRenderer.h"

class Arena : public WorldComponent
{
public:
  // Raised when arena fight starts
  Event OnBattleStart;

  Arena(GameObject &associatedObject, float width, float height);

  Arena(GameObject &associatedObject, std::shared_ptr<SpriteRenderer> backgroundRenderer);

  virtual ~Arena() {}
  void Render() override;

  RenderLayer GetRenderLayer() override { return RenderLayer::Debug; }

  // Detect if battle is over
  void CheckBattleOver();

  // Arena size's width
  const float width;

  // Arena size's height
  const float height;

private:
  // Reference to background sprite renderer
  std::weak_ptr<SpriteRenderer> weakBackgroundRenderer;
};

#endif