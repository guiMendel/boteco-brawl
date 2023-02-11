#ifndef __CHARACTER_UI_OPTION__
#define __CHARACTER_UI_OPTION__

#include "UIComponent.h"
#include "FallDeath.h"
#include "UIImage.h"
#include "UIText.h"
#include "UIContainer.h"
#include "BrawlPlayer.h"
#include "Animator.h"

class CharacterUIOption : public UIComponent
{
public:
  // Sets the character of a player
  using CharacterSetter = std::function<void(std::shared_ptr<BrawlPlayer>)>;
  using AnimationSetter = std::function<void(std::shared_ptr<Animator>)>;

  CharacterUIOption(
      GameObject &associatedObject,
      std::string characterBillTextPath,
      AnimationSetter setAnimation,
      CharacterSetter characterSetter);

  virtual ~CharacterUIOption() {}

  std::string characterBillTextPath;
  AnimationSetter setAnimation;
  CharacterSetter characterSetter;
};

#endif