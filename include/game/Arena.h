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

  Arena(GameObject &associatedObject);

  virtual ~Arena() {}
  void Render() override;
  void Awake() override;

  RenderLayer GetRenderLayer() override { return RenderLayer::Debug; }

  // Detect if battle is over
  void CheckBattleOver();

  float GetWidth() const;
  float GetHeight() const;

protected:
  // Set up the arena space
  virtual void InitializeArena() = 0;

  // Arena size's width, in units
  float width{0};

  // Arena size's height, in units
  float height{0};
};

#endif