#include "UIContent.h"
#include "UIContainer.h"

using namespace std;

UIContent::UIContent(Canvas &canvas, string name, int gameSceneId, int id)
    : UIObject(canvas, name, gameSceneId, id) {}

UIContent::UIContent(Canvas &canvas, string name, shared_ptr<UIContainer> parent)
    : UIObject(canvas, name, parent) {}

void UIContent::CascadeDown(function<void(GameObject &)> callback, bool) { callback(*this); }
