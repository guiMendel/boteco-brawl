#ifndef __CHARACTER_UI_OPTION__
#define __CHARACTER_UI_OPTION__

#include "UIComponent.h"
#include "FallDeath.h"
#include "UIImage.h"
#include "UIText.h"
#include "UIContainer.h"

class CharacterUIOption : public UIComponent
{
public:
  CharacterUIOption(
      GameObject &associatedObject,
      std::string characterBillTextPath);
      
  virtual ~CharacterUIOption() {}

  std::string characterBillTextPath;
};

#endif