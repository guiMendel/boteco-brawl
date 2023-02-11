#include "CharacterUIOption.h"

CharacterUIOption::CharacterUIOption(
    GameObject &associatedObject,
    std::string characterBillTextPath)
    : UIComponent(associatedObject), characterBillTextPath(characterBillTextPath) {}
