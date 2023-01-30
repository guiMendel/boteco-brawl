#include "Canvas.h"

using namespace std;

Canvas::Canvas(GameObject &associatedObject, Space space)
    : WorldComponent(associatedObject), space(space), root(make_shared<UIContainer>(*this, "UIRoot")) {}
