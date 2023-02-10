#include "Circle.h"
#include "Debug.h"

using namespace std;
using namespace Helper;

Circle::Circle(Vector2 center, float radius) : Shape(center), radius(radius) {}

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

bool Circle::Contains(const Vector2 &point) const
{
  return Vector2::SqrDistance(point, center) <= radius * radius;
}

float Circle::GetArea() const { return M_PI * radius * radius; }

Circle::operator string() const
{
  return "{ x: " + to_string(center.x) + ", y: " + to_string(center.y) + ", r: " + to_string(radius) + " }";
}

Circle operator*(float value, const Circle &circle) { return circle * value; }
Circle operator/(float value, const Circle &circle) { return Circle(value / circle.center, value / circle.radius); }

float Circle::GetMaxDimension() { return radius * 2; }
float Circle::GetMinDimension() { return radius * 2; }

void Circle::Scale(Vector2 scale)
{
  // Get absolute values
  scale = scale.GetAbsolute();

  Assert(scale.x == scale.y, "Scale for a circle must have the same absolute values for both x and y coordinates");

  radius *= scale.x;
}

Vector2 Circle::SamplePoint() const
{
  return Vector2::Angled(RandomRange(0.0f, 2 * M_PI), RandomRange(0.0f, radius)) + center;
}

void Circle::DebugDrawAt(Vector2 position, Color color)
{
  Debug::DrawCircle(*this + position, color);
}
