#ifndef __RECTANGLE__
#define __RECTANGLE__

#include <initializer_list>
#include <vector>
#include <SDL.h>
#include <iostream>
#include "Vector2.h"

class Rectangle
{
public:
  // Used for initializing rectangle with top left coordinates
  enum TopLeftInitializerLabel
  {
    TopLeftInitialize
  };

  Vector2 center;

  // Width
  float width;

  // Height
  float height;

  // === CONSTRUCTORS

  Rectangle(const Vector2 &coordinates, float width, float height);
  Rectangle(float x, float y, float width, float height);
  Rectangle();
  Rectangle(const Rectangle &other);
  Rectangle(const Rectangle &&other);
  Rectangle(const SDL_Rect &rect);
  // Interprets coordinates as top left
  Rectangle(TopLeftInitializerLabel, const Vector2 &coordinates, float width, float height);

  // === OPERATIONS

  Rectangle operator=(const Rectangle &other);
  Rectangle operator+(const Vector2 &vector) const;
  Rectangle operator-(const Vector2 &vector) const;
  Rectangle operator*(float value) const;
  Rectangle operator/(float value) const;

  // Indicates if a given coordinate is contained by the rectangle
  bool Contains(const Vector2 &vector) const;

  Vector2 TopLeft(float pivoted = 0.0f) const;
  Vector2 BottomLeft(float pivoted = 0.0f) const;
  Vector2 BottomRight(float pivoted = 0.0f) const;
  Vector2 TopRight(float pivoted = 0.0f) const;

  std::vector<Vector2> Vertices(float pivoted = 0.0f) const;

  // Convert to sdl rect
  explicit operator SDL_Rect() const;

  explicit operator Vector2() const;

  explicit operator std::string() const;
};

Rectangle operator*(float value, const Rectangle &rectangle);
Rectangle operator/(float value, const Rectangle &rectangle);
std::ostream &operator<<(std::ostream &stream, const Rectangle &rectangle);

#endif