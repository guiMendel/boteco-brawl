#ifndef __DEBUG__
#define __DEBUG__

#include "Vector2.h"

class Debug
{
public:
  static void DrawPoint(Vector2 point, bool convertToScreen = true);
};

#endif