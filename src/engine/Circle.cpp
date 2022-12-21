#include "Circle.h"

using namespace std;

Circle::Circle(Vector2 center, float radius) : center(center), radius(radius) {}

Circle::Circle(float radius) : Circle(Vector2::Zero(), radius) {}

Circle::Circle() : Circle(Vector2::Zero(), 0) {}

Circle::Circle(const Circle &other) : Circle(other.center, other.radius) {}

Circle::Circle(const Circle &&other) : Circle(other.center, other.radius) {}

// === OPERATIONS

Circle Circle::operator=(const Circle &other)
{
  center = other.center;
  radius = other.radius;
  return *this;
}

Circle Circle::operator+(const Vector2 &vector) const
{
  return Circle(center + vector, radius);
}

Circle Circle::operator-(const Vector2 &vector) const
{
  return Circle(center - vector, radius);
}

Circle Circle::operator*(float value) const { return Circle(center * value, radius * value); }
Circle Circle::operator/(float value) const { return Circle(center / value, radius / value); }

Circle operator+(const Vector2 &vector, const Circle &circle) { return circle + vector; }
Circle operator-(const Vector2 &vector, const Circle &circle) { return circle + -vector; }

Circle Circle::operator+=(const Vector2 &vector) { return *this = *this + vector; }

Circle Circle::operator-=(const Vector2 &vector) { return *this = *this - vector; }

// Indicates if a given coordinate is contained by the Circle
bool Circle::Contains(const Vector2 &vector) const
{
  return Vector2::Distance(center, vector) <= radius;
}

Circle::operator Vector2() const { return Vector2{center}; }

Circle::operator string() const
{
  return "{ x: " + to_string(center.x) + ", y: " + to_string(center.y) + ", r: " + to_string(radius) + " }";
}

ostream &operator<<(ostream &stream, const Circle &circle) { return stream << (string)circle; }

Circle operator*(float value, const Circle &circle) { return circle * value; }
Circle operator/(float value, const Circle &circle) { return Circle(value / circle.center, value / circle.radius); }
