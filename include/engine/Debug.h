#ifndef __DEBUG__
#define __DEBUG__

#include "Vector2.h"
#include "Circle.h"
#include "Rectangle.h"

class Debug
{
public:
  static void DrawPoint(Vector2 point);
  static void DrawCircle(Circle circle);
  static void DrawBox(Rectangle rectangle, float rotation = 0);
};

#endif