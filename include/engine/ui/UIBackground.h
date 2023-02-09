#ifndef __UI_BACKGROUND__
#define __UI_BACKGROUND__

#include "UIComponent.h"
#include "UIContainer.h"

// Maps input from some source (player or AI) to the the creation and dispatch of the corresponding actions
class UIBackground : public UIComponent
{
public:
  // How quickly the text size multiplier decays back to 1
  static const float textSizeMultiplierDecay;

  UIBackground(GameObject &associatedObject, Color color);
  virtual ~UIBackground() {}

  void Render() override;

  Color color;
};

#endif