#include "UIEvent.h"

using namespace std;
using namespace Helper;

UIMouseEvent::UIMouseEvent(Type type, Vector2 screenPosition) : screenPosition(screenPosition), type(type)
{
    Assert(
        type == Type::OnMouseClick ||
            type == Type::OnMouseEnter ||
            type == Type::OnMouseLeave ||
            type == Type::OnMouseOver,
        "Invalid type for UI Mouse Event");
}
