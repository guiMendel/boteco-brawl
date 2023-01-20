#ifndef __RENDER_LAYER__
#define __RENDER_LAYER__

// Layers on which to render. The Last layers will be rendered on top of the first layers
enum class RenderLayer
{
  Background,
  Default,
  Characters,
  Projectiles,
  VFX,
  Foreground,
  UI,
  Debug,
  None
};

#endif