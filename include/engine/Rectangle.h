#ifndef __RECTANGLE__
#define __RECTANGLE__

#include <initializer_list>
#include <vector>
#include <SDL.h>
#include "Vector2.h"

class Rectangle
{
public:
  // X coordinate (rect's center)
  float x;

  // Y coordinate (rect's center)
  float y;

  // Width
  float width;

  // Height
  float height;

  // === CONSTRUCTORS

  Rectangle(float x, float y, float width, float height) : x(x), y(y), width(width), height(height) {}

  Rectangle(const Vector2 &coordinates, float width, float height) : x(coordinates.x), y(coordinates.y), width(width), height(height) {}

  Rectangle() : x(0), y(0), width(0), height(0) {}

  Rectangle(std::initializer_list<float> list)
      : x(*list.begin()), y(*(list.begin() + 1)), width(*(list.begin() + 2)), height(*(list.begin() + 3)) {}

  Rectangle(const Rectangle &other) : x(other.x), y(other.y), width(other.width), height(other.height) {}

  Rectangle(const Rectangle &&other) : x(other.x), y(other.y), width(other.width), height(other.height) {}

  // === OPERATIONS

  Rectangle operator=(const Rectangle &other)
  {
    x = other.x;
    y = other.y;
    width = other.width;
    height = other.height;
    return *this;
  }

  Rectangle operator+(const Vector2 &vector) const
  {
    return Rectangle(x + vector.x, y + vector.y, width, height);
  }

  Rectangle operator-(const Vector2 &vector) const
  {
    return Rectangle(x - vector.x, y - vector.y, width, height);
  }

  Rectangle operator=(const Vector2 &vector)
  {
    x = vector.x;
    y = vector.y;
    return *this;
  }

  Rectangle operator+=(const Vector2 &vector) { return *this = *this + vector; }

  Rectangle operator-=(const Vector2 &vector) { return *this = *this - vector; }

  Vector2 Coordinates() const { return Vector2(x, y); }

  // Indicates if a given coordinate is contained by the rectangle
  bool Contains(const Vector2 &vector) const
  {
    return vector.x >= x && vector.x <= x + width && vector.y >= y && vector.y <= y + height;
  }

  // Pivot's a point around a rectangles center
  Vector2 Pivot(Vector2 point, float radians) const
  {
    return (point - Center()).Rotated(radians) + Center();
  }

  Vector2 Center() const { return Vector2(x, y); }
  Vector2 TopLeft(float pivoted = 0.0f) const
  {
    return Pivot(Vector2(x - width / 2, y - height / 2), pivoted);
  }
  Vector2 BottomLeft(float pivoted = 0.0f) const
  {
    return Pivot(Vector2(x - width / 2, y + height / 2), pivoted);
  }
  Vector2 BottomRight(float pivoted = 0.0f) const
  {
    return Pivot(Vector2(x + width / 2, y + height / 2), pivoted);
  }
  Vector2 TopRight(float pivoted = 0.0f) const
  {
    return Pivot(Vector2(x + width / 2, y - height / 2), pivoted);
  }

  std::vector<Vector2> Vertices(float pivoted = 0.0f) const
  {
    return {TopRight(pivoted), BottomRight(pivoted), BottomLeft(pivoted), TopLeft(pivoted)};
  }

  // Convert to sdl rect
  operator SDL_Rect() const { return SDL_Rect{(int)x, (int)y, (int)width, (int)height}; }

  explicit operator Vector2() const { return Vector2{x, y}; }

  explicit operator std::string() const
  {
    return "{ x: " + std::to_string(x) + ", y: " + std::to_string(y) + ", w: " + std::to_string(width) + ", h: " + std::to_string(height) + " }";
  }
};

#endif