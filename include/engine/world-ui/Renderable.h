#ifndef __RENDERABLE__
#define __RENDERABLE__

#include "RenderLayer.h"

class GameScene;

// Defines a data type which can be rendered to the screen
class Renderable
{
  friend class GameScene;

public:
  // In which render layer this component is
  // If None, then it's Render method will never be called
  virtual RenderLayer GetRenderLayer() { return RenderLayer::None; }

  // The order in which to render this component in it's layer (higher numbers are shown on top)
  virtual int GetRenderOrder() { return 0; }

protected:
  // Registers this component's render layer if it is not None
  virtual void RegisterLayer() = 0;

private:
  // Whether this data type should be rendered at the moment of this call
  virtual bool ShouldRender() { return true; }

  // Called once per frame to render to the screen
  virtual void Render() {}
};

#endif