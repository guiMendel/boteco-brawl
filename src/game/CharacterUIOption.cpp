#include "CharacterUIOption.h"

using namespace std;

CharacterUIOption::CharacterUIOption(
    GameObject &associatedObject,
    string characterBillTextPath,
    CharacterSetter characterSetter)
    : UIComponent(associatedObject), characterBillTextPath(characterBillTextPath), characterSetter(characterSetter) {}
