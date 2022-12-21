#ifndef __CIRCLE__
#define __CIRCLE__

#include "Vector2.h"

class Circle
{
public:
  // Centers coords
  Vector2 center;

  // Radius
  float radius;

  // === CONSTRUCTORS

  Circle(Vector2 center, float radius);
  Circle(float radius);
  Circle();
  Circle(const Circle &other);
  Circle(const Circle &&other);

  // === OPERATIONS

  Circle operator=(const Circle &other);

  Circle operator+(const Vector2 &vector) const;

  Circle operator-(const Vector2 &vector) const;

  Circle operator*(float value) const;

  Circle operator/(float value) const;

  Circle operator+=(const Vector2 &vector);

  Circle operator-=(const Vector2 &vector);

  // Indicates if a given coordinate is contained by the Circle
  bool Contains(const Vector2 &vector) const;

  explicit operator Vector2() const;

  explicit operator std::string() const;
};

Circle operator*(float value, const Circle &circle);
Circle operator/(float value, const Circle &circle);
Circle operator+(const Vector2 &vector, const Circle &circle);
Circle operator-(const Vector2 &vector, const Circle &circle);
std::ostream &operator<<(std::ostream &stream, const Vector2 &vector);

#endif