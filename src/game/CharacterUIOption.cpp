#include "CharacterUIOption.h"

using namespace std;

CharacterUIOption::CharacterUIOption(
    GameObject &associatedObject,
    string characterBillTextPath,
    AnimationSetter setAnimation,
    CharacterSetter characterSetter)
    : UIComponent(associatedObject),
      characterBillTextPath(characterBillTextPath),
      setAnimation(setAnimation),
      characterSetter(characterSetter) {}
