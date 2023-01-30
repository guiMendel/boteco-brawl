#include "UIRoot.h"

using namespace std;

const string UIRoot::rootName{"Root"};

UIRoot::UIRoot(Canvas &canvas, int gameSceneId, int id)
    : UIContainer(canvas, rootName, gameSceneId, id) {}

UIRoot::UIRoot(Canvas &canvas)
    : UIContainer(canvas, rootName, nullptr) {}

shared_ptr<GameObject> UIRoot::InternalGetParent() const { return RequirePointerCast<GameObject>(canvas.GetShared()); }
