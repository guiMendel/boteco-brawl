#include "Canvas.h"

Canvas::Canvas(GameObject &associatedObject, Space space)
    : WorldComponent(associatedObject), space(space), root(*this) {}
