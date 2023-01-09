#ifndef __DEBUG__
#define __DEBUG__

#include "Vector2.h"
#include "Circle.h"
#include "Rectangle.h"
#include "Color.h"

class Debug
{
public:
  static void DrawPoint(Vector2 point, Color color = Color::Green());
  static void DrawCircle(Circle circle, Color color = Color::Green());
  static void DrawBox(Rectangle rectangle, Color color = Color::Green());
};

#endif