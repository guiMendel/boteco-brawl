#ifndef __CHARACTER_UI_OPTION__
#define __CHARACTER_UI_OPTION__

#include "UIComponent.h"
#include "FallDeath.h"
#include "UIImage.h"
#include "UIText.h"
#include "UIContainer.h"
#include "BrawlPlayer.h"

class CharacterUIOption : public UIComponent
{
public:
  // Sets the character of a player
  using CharacterSetter = std::function<void(std::shared_ptr<BrawlPlayer>)>;

  CharacterUIOption(
      GameObject &associatedObject,
      std::string characterBillTextPath,
      CharacterSetter characterSetter);

  virtual ~CharacterUIOption() {}

  std::string characterBillTextPath;
  CharacterSetter characterSetter;
};

#endif