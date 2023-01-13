#ifndef __DEBUG__
#define __DEBUG__

#include "Vector2.h"
#include "Circle.h"
#include "Rectangle.h"
#include "Color.h"

class Debug
{
public:
  // Draws a point, given it's global position in game units
  static void DrawPoint(Vector2 point, Color color = Color::Green());

  // Draws a circle, given it's global position and dimensions in game units
  static void DrawCircle(Circle circle, Color color = Color::Green());

  // Draws a rectangle, given it's global position and dimensions in game units
  static void DrawBox(Rectangle rectangle, Color color = Color::Green());

  // Draws a line, given it's global start and end positions in game units
  static void DrawLine(Vector2 start, Vector2 end, Color color = Color::Green());

  // Draws a line with an arrow, given it's global start and end positions in game units
  // Allows setting head size in units
  static void DrawArrow(Vector2 start, Vector2 end, Color color = Color::Green(), float headSize = 1, float headArcAngle = M_PI / 2);
};

#endif